/*
 * ttn_geolocation_gnss.ino
 * Copyright (C) 2023 Seeed K.K.
 * MIT License
 */

////////////////////////////////////////////////////////////////////////////////
// Includes

#include <Arduino.h>

#include <LbmWm1110.hpp>
#include <Lbmx.hpp>

#include <Lbm_packet.hpp>

////////////////////////////////////////////////////////////////////////////////
// Constants

static constexpr smtc_modem_region_t REGION = SMTC_MODEM_REGION_AS_923_GRP1;
static const uint8_t CUSTOM_ADR[SMTC_MODEM_CUSTOM_ADR_DATA_LENGTH] = { 5, 5, 5, 5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 3, 3 };
static constexpr uint8_t TRANS_NUMBER = 1;

static constexpr uint32_t TIME_SYNC_VALID_TIME = 60 * 60 * 24;  // [sec.]

static constexpr uint32_t GNSS_SCAN_PERIOD = 180;    // [sec.]
static constexpr gnss_mw_mode_t GNSS_SCAN_MODE = GNSS_MW_MODE_MOBILE;

static constexpr uint32_t EXECUTION_PERIOD = 50;    // [msec.]

static constexpr uint32_t FIRST_UPLINK_DELAY = 20;  // [sec.]
static constexpr uint32_t UPLINK_PERIOD = 15;       // [sec.]

static constexpr uint8_t lorawan_port = 192;

////////////////////////////////////////////////////////////////////////////////
// Variables
uint32_t position_period = GNSS_SCAN_PERIOD*1000;   // [msec.]
uint32_t gnss_scan_timeout = 120000; // [msec.]
bool gnss_scan_end = false;

static LbmWm1110& lbmWm1110 = LbmWm1110::getInstance();

////////////////////////////////////////////////////////////////////////////////
// Functions

static void PrintGnssScan(const gnss_mw_event_data_scan_done_t* data)
{
    if (data != nullptr)
    {
        printf("SCAN_DONE info:\n");
        printf("-- token: 0x%02X\n", data->token);
        printf("-- is_valid: %d\n", data->is_valid);
        printf("-- number of valid scans: %u\n", data->nb_scans_valid);
        for (uint8_t i = 0; i < data->nb_scans_valid; ++i)
        {
            printf("-- scan[%d][%lu] (%u SV - %d): ", i, data->scans[i].timestamp, data->scans[i].nb_svs, data->scans[i].nav_valid);
            for (uint8_t j = 0; j < data->scans[i].nav_size; ++j)
            {
                printf("%02X", data->scans[i].nav[j]);
            }
            printf("\n");
            for (uint8_t j = 0; j < data->scans[i].nb_svs; ++j)
            {
                printf("   SV_ID %u:\t%ddB\n", data->scans[i].info_svs[j].sv_id, data->scans[i].info_svs[j].cnr);
            }
        }
        printf("-- power consumption: %lu uah\n", data->power_consumption_uah);
        printf("-- mode: %d\n", data->context.mode);
        printf("-- assisted: %d\n", data->context.assisted);
        if (data->context.assisted)
        {
            printf("-- aiding position: (%.6f, %.6f)\n", data->context.aiding_position_latitude, data->context.aiding_position_longitude);
        }
        printf("-- almanac CRC: 0X%08lX\n", data->context.almanac_crc);
        printf("-- almanac update required: %d\n", data->context.almanac_update_required);
        printf("-- indoor detected: %d\n", data->indoor_detected);
        if (data->aiding_position_check_size > 0)
        {
            printf("-- APC (%u): ", data->aiding_position_check_size);
            for (uint8_t i = 0; i < data->aiding_position_check_size; ++i)
            {
                printf("%02X", data->aiding_position_check_msg[i]);
            }
            printf("\n");
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// MyLbmxEventHandlers

class MyLbmxEventHandlers : public LbmxEventHandlers
{
protected:
    void reset(const LbmxEvent& event) override;
    void joined(const LbmxEvent& event) override;
    void alarm(const LbmxEvent& event) override;
    void time(const LbmxEvent& event) override;
    void almanacUpdate(const LbmxEvent& event) override;  
    void txDone(const LbmxEvent& event);   
    void downData(const LbmxEvent& event) override;
    void gnssScanDone(const LbmxEvent& event) override;
    void gnssTerminated(const LbmxEvent& event) override;
    void gnssErrorNoTime(const LbmxEvent& event) override;
    void gnssErrorAlmanacUpdate(const LbmxEvent& event) override;
    void gnssScanStopped(const LbmxEvent& event) override;

};

void MyLbmxEventHandlers::reset(const LbmxEvent& event)
{
    if (LbmxEngine::setRegion(REGION) != SMTC_MODEM_RC_OK) abort();
    if (LbmxEngine::setOTAA(app_param.lora_info.DevEui, app_param.lora_info.JoinEui, app_param.lora_info.AppKey) != SMTC_MODEM_RC_OK) abort();

    if (LbmxDeviceManagement::setInfoInterval(SMTC_MODEM_DM_INFO_INTERVAL_IN_DAY, 1) != SMTC_MODEM_RC_OK) abort();
    if (LbmxDeviceManagement::setInfoFields({ SMTC_MODEM_DM_FIELD_ALMANAC_STATUS }) != SMTC_MODEM_RC_OK) abort();

    printf("Join the LoRaWAN network.\n");
    if (LbmxEngine::joinNetwork() != SMTC_MODEM_RC_OK) abort();
}

void MyLbmxEventHandlers::joined(const LbmxEvent& event)
{
    if (smtc_modem_time_set_sync_interval_s(TIME_SYNC_VALID_TIME / 3) != SMTC_MODEM_RC_OK) abort();     // keep call order
    if (smtc_modem_time_set_sync_invalid_delay_s(TIME_SYNC_VALID_TIME) != SMTC_MODEM_RC_OK) abort();    // keep call order

    printf("Start time sync.\n");
    if (smtc_modem_time_start_sync_service(0, SMTC_MODEM_TIME_ALC_SYNC) != SMTC_MODEM_RC_OK) abort();

    printf("Start the alarm event.\n");
    if (LbmxEngine::startAlarm(FIRST_UPLINK_DELAY) != SMTC_MODEM_RC_OK) abort();

}
void MyLbmxEventHandlers::alarm(const LbmxEvent& event)
{

    static uint32_t counter = 0;
    app_task_lora_tx_engine();
    if (LbmxEngine::startAlarm(UPLINK_PERIOD) != SMTC_MODEM_RC_OK) abort();
}
void MyLbmxEventHandlers::almanacUpdate(const LbmxEvent& event)
{
    if( event.event_data.almanac_update.status == SMTC_MODEM_EVENT_ALMANAC_UPDATE_STATUS_REQUESTED )
    {
        printf( "Almanac update is not completed: sending new request\n" );
    }
    else
    {
        printf( "Almanac update is completed\n" );
    }
}
void MyLbmxEventHandlers::txDone(const LbmxEvent& event)
{
    static uint32_t uplink_count = 0;

    if( event.event_data.txdone.status == SMTC_MODEM_EVENT_TXDONE_CONFIRMED )
    {
        uplink_count++;
    }

    uint32_t tick = smtc_modem_hal_get_time_in_ms( );
}
void MyLbmxEventHandlers::time(const LbmxEvent& event)
{
    if (event.event_data.time.status == SMTC_MODEM_EVENT_TIME_NOT_VALID) return;

    static bool first = true;
    if (first)
    {
        printf("time sync ok\r\n");
        if( is_first_time_sync == false )
        {
            is_first_time_sync = true;
        }
        // Configure ADR and transmissions
        if (smtc_modem_adr_set_profile(0, SMTC_MODEM_ADR_PROFILE_CUSTOM, CUSTOM_ADR) != SMTC_MODEM_RC_OK) abort();
        if (smtc_modem_set_nb_trans(0, TRANS_NUMBER) != SMTC_MODEM_RC_OK) abort();
        if (smtc_modem_connection_timeout_set_thresholds(0, 0, 0) != SMTC_MODEM_RC_OK) abort();

        first = false;
    }
}

void MyLbmxEventHandlers::downData(const LbmxEvent& event)
{
    printf("Downlink received:\n");
    printf("  - LoRaWAN Fport = %d\n", event.event_data.downdata.fport);
    printf("  - Payload size  = %d\n", event.event_data.downdata.length);
    printf("  - RSSI          = %d dBm\n", event.event_data.downdata.rssi - 64);
    printf("  - SNR           = %d dB\n", event.event_data.downdata.snr / 4);

    if (event.event_data.downdata.length != 0)
    {
        gnss_mw_handle_downlink(event.event_data.downdata.fport, event.event_data.downdata.data, event.event_data.downdata.length);
    }
}

void MyLbmxEventHandlers::gnssScanDone(const LbmxEvent& event)
{
    printf("----- GNSS - %s -----\n", event.getGnssEventString(GNSS_MW_EVENT_SCAN_DONE).c_str());

    gnss_mw_event_data_scan_done_t eventData;
    gnss_mw_get_event_data_scan_done(&eventData);
    PrintGnssScan(&eventData);
    float latitude_dif, longitude_dif;
    latitude_dif = fabs( eventData.context.aiding_position_latitude - app_task_gnss_aiding_position_latitude );
    longitude_dif = fabs( eventData.context.aiding_position_longitude - app_task_gnss_aiding_position_longitude );

    /* Store the new assistance position only if the difference is greater than the conversion error */
    if(( latitude_dif > ( float ) 0.03 ) || ( longitude_dif > ( float ) 0.03 ))
    {
        app_task_gnss_aiding_position_latitude  = eventData.context.aiding_position_latitude;
        app_task_gnss_aiding_position_longitude = eventData.context.aiding_position_longitude;
        // TODO, save aiding position to nvds
        int32_t lat_temp = app_task_gnss_aiding_position_latitude * 1000000;
        int32_t long_temp = app_task_gnss_aiding_position_longitude * 1000000;
        printf( "New assistance position stored: %d, %d\r\n", lat_temp, long_temp );

    }
    if (eventData.context.almanac_update_required)
    {
        const uint8_t dmAlmanacStatus = SMTC_MODEM_DM_FIELD_ALMANAC_STATUS;
        if (LbmxDeviceManagement::requestUplink({ SMTC_MODEM_DM_FIELD_ALMANAC_STATUS }) != SMTC_MODEM_RC_OK) abort();
    }
}

void MyLbmxEventHandlers::gnssTerminated(const LbmxEvent& event)
{
    printf("----- GNSS - %s -----\n", event.getGnssEventString(GNSS_MW_EVENT_TERMINATED).c_str());

    gnss_mw_event_data_terminated_t eventData;
    gnss_mw_get_event_data_terminated(&eventData);
    printf("TERMINATED info:\n");
    printf("-- number of scans sent: %u\n", eventData.nb_scans_sent);
    printf("-- aiding position check sent: %d\n", eventData.aiding_position_check_sent);
}

void MyLbmxEventHandlers::gnssErrorNoTime(const LbmxEvent& event)
{
    printf("----- GNSS - %s -----\n", event.getGnssEventString(GNSS_MW_EVENT_ERROR_NO_TIME).c_str());

    if (smtc_modem_time_trigger_sync_request(0) != SMTC_MODEM_RC_OK) abort();
    mw_gnss_event_state = GNSS_MW_EVENT_ERROR_NO_TIME;
}

void MyLbmxEventHandlers::gnssErrorAlmanacUpdate(const LbmxEvent& event)
{
    printf("----- GNSS - %s -----\n", event.getGnssEventString(GNSS_MW_EVENT_ERROR_ALMANAC_UPDATE).c_str());

    const uint8_t dmAlmanacStatus = SMTC_MODEM_DM_FIELD_ALMANAC_STATUS;
    if (LbmxDeviceManagement::requestUplink({ SMTC_MODEM_DM_FIELD_ALMANAC_STATUS }) != SMTC_MODEM_RC_OK) abort();
    mw_gnss_event_state = GNSS_MW_EVENT_ERROR_ALMANAC_UPDATE;
}

void MyLbmxEventHandlers::gnssScanStopped(const LbmxEvent& event)
{
    gnss_scan_end = true;
    gnss_mw_custom_clear_scan_busy();
}

////////////////////////////////////////////////////////////////////////////////
// ModemEventHandler

static void ModemEventHandler()
{
    static LbmxEvent event;
    static MyLbmxEventHandlers handlers;

    while (event.fetch())
    {
        printf("----- %s -----\n", event.getEventString().c_str());

        handlers.invoke(event);
    }
}

////////////////////////////////////////////////////////////////////////////////
// setup and loop

void setup()
{

    delay(1000);
    printf("\n---------- STARTUP ----------\n");

    default_param_load();
    sensor_init_detect();

    lbmWm1110.attachGnssPrescan([](void* context){ digitalWrite(PIN_GNSS_LNA, HIGH); });
    lbmWm1110.attachGnssPostscan([](void* context){ digitalWrite(PIN_GNSS_LNA, LOW); });
    lbmWm1110.begin();

    app_gps_scan_init();
    // /* Initialize GNSS middleware */
    gnss_mw_init( lbmWm1110.getRadio(), stack_id );
    gnss_mw_custom_enable_copy_send_buffer();
    gnss_mw_set_constellations( GNSS_MW_CONSTELLATION_GPS_BEIDOU );

    LbmxEngine::begin(lbmWm1110.getRadio(), ModemEventHandler);

    LbmxEngine::printVersions(lbmWm1110.getRadio());
}

void loop()
{
    static uint32_t now_time = 0;
	static uint32_t start_scan_time = 0;  
    bool result = false;  
    uint32_t sleepTime = LbmxEngine::doWork();
    if(is_first_time_sync == true)
    {
        now_time = smtc_modem_hal_get_time_in_ms( );
        if(sleepTime > 500)
        {
            if(position_period<120000) position_period = 120000;
            if(now_time - start_scan_time > position_period ||(start_scan_time == 0))
            {
                printf("start scan gnss\r\n");
                app_gps_scan_start();
                gnss_scan_end = false;
                start_scan_time = smtc_modem_hal_get_time_in_ms( );
            }
            // ledOff(LED_BUILTIN);
            if(((smtc_modem_hal_get_time_in_ms( ) - start_scan_time > gnss_scan_timeout)||(gnss_scan_end)) &&(gps_scan_status == 2))
            {
                result = app_gps_get_results( tracker_gps_scan_data, &tracker_gps_scan_len );
                if( result )
                {
                    app_gps_display_results( );
                }
                printf("stop scan gnss\r\n");
                app_gps_scan_stop( );
                sensor_datas_get();
                //send data to LoRaWAN
                // raw datas
                for( uint8_t i = 0; i < gnss_mw_custom_send_buffer_num; i++ )
                {
                    for( uint8_t j = 0; j < gnss_mw_custom_send_buffer_len[i]; j++ )
                    {
                        app_task_lora_tx_queue( gnss_mw_custom_send_buffer[i], gnss_mw_custom_send_buffer_len[i], false, false );
                    }
                }
                //TODO  user can compose any package they want
                app_lora_set_port(lorawan_port);  //set upload data port

            }
        }
        sleepTime = smtc_modem_hal_get_time_in_ms( )-now_time;
    }
    delay(min(sleepTime, EXECUTION_PERIOD));
}

////////////////////////////////////////////////////////////////////////////////
