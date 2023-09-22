/*
 * default_firmware.ino
 * Copyright (C) 2023 Seeed K.K.
 * MIT License
 */

////////////////////////////////////////////////////////////////////////////////
// Includes

#include <Arduino.h>

#include <LbmWm1110.hpp>
#include <Lbmx.hpp>

#include <Lbm_packet.hpp>


/* most important things:
// Please follow local regulations to set lorawan duty cycle limitations => smtc_modem_set_region_duty_cycle()
// 
//  Make sure the 'sleepTime' is greater than the time required to run the code.Otherwise, LoRaWAN will run incorrectly
//
//  USER TODO:
//  1.Redefine parameters   =>      'DEV_EUI','JOIN_EUI','APP_KEY'
//  2.Modify parameters     =>      'REGION'
//  3.Comment code call     =>      'init_current_lorawan_param'
//  4.Modify parameters     =>      'position_period'
//  5.Modify parameters     =>      'sensor_read_period'
//
*/


////////////////////////////////////////////////////////////////////////////////
// Types

enum class StateType
{
    Startup,
    Joining,
    Joined,
    Failed,
};

////////////////////////////////////////////////////////////////////////////////
// Constants

static constexpr uint32_t TIME_SYNC_VALID_TIME = 60 * 60 * 24;  // [sec.] 
static constexpr uint32_t FIRST_UPLINK_DELAY = 20;  // [sec.]
static constexpr uint32_t UPLINK_PERIOD = 10;       // [sec.]


static constexpr uint32_t EXECUTION_PERIOD = 60;    // [msec.]

////////////////////////////////////////////////////////////////////////////////
// Variables
bool gnss_scan_end = false;

uint32_t position_period = 300*1000;   // [msec.]
uint32_t gnss_scan_timeout = 120000; // [msec.]

uint32_t consume_time = 0;

uint8_t DEV_EUI[8];
uint8_t JOIN_EUI[8];
uint8_t APP_KEY[16];
static smtc_modem_region_t REGION = SMTC_MODEM_REGION_EU_868;

uint32_t sensor_read_period = 300*1000;   // [msec.]
uint8_t button_press_flag = 0;
uint8_t button_trig_position = 0;
uint8_t button_trig_collect = 0;


static LbmWm1110& lbmWm1110 = LbmWm1110::getInstance();
static StateType state = StateType::Startup;

////////////////////////////////////////////////////////////////////////////////
void print_current_lorawan_param(void)
{
    printf("DevEui:\r\n");
    for(uint8_t u8i = 0;u8i < 8; u8i++)
    {
        printf("%02x ",DEV_EUI[u8i]);
    }
    printf("\r\nJoinEui:\r\n");
    for(uint8_t u8i = 0;u8i < 8; u8i++)
    {
        printf("%02x ",JOIN_EUI[u8i]);
    }
    printf("\r\nAppKey:\r\n");
    for(uint8_t u8i = 0;u8i < 16; u8i++)
    {
        printf("%02x ",APP_KEY[u8i]);
    }
    printf("\r\n");    

    position_period = app_append_param.position_interval*60*1000;
    sensor_read_period = app_append_param.sample_interval*60*1000;

    printf("position_period:%umin\r\n",app_append_param.position_interval);
    printf("sensor_read_period:%umin\r\n",app_append_param.sample_interval);

}
void init_current_lorawan_param(void)
{
    memcpy(DEV_EUI, app_param.lora_info.DevEui, sizeof(DEV_EUI));
    memcpy(JOIN_EUI, app_param.lora_info.JoinEui, sizeof(JOIN_EUI));
    memcpy(APP_KEY, app_param.lora_info.AppKey, sizeof(APP_KEY));
    REGION = sensecap_lorawan_region();

    print_current_lorawan_param();
}

//irq callback----------------------------------------------------
void user_button_irq_callback(void)
{
    static uint32_t key_press_cnt = 0;
    static uint32_t key_release_cnt = 0;    
    static uint8_t btn_state = 0; 

    if( btn_state == 0 )
    {
        if( 0 == digitalRead( PIN_BUTTON1 ))
        {
            btn_state = 1;
            key_press_cnt = smtc_modem_hal_get_time_in_ms( );
        }
    }
    else if( btn_state == 1 )
    {
        if( 1 == digitalRead( PIN_BUTTON1 ))
        {
            btn_state = 0;
            key_release_cnt = smtc_modem_hal_get_time_in_ms( );
            if(( key_release_cnt - key_press_cnt ) > 10 ) //press
            {
                button_press_flag = 1;
                button_trig_position = 1;
                button_trig_collect = 1;
                key_press_cnt = 0;
                state_all = state_all|TRACKER_STATE_BIT0_SOS;
                
            }
        }
    }
}

void user_button_init(void)
{
    pinMode(PIN_BUTTON1, INPUT);
    attachInterrupt(PIN_BUTTON1, user_button_irq_callback, ISR_DEFERRED | CHANGE); 
}


// MyLbmxEventHandlers
class MyLbmxEventHandlers : public LbmxEventHandlers
{
protected:
    void reset(const LbmxEvent& event) override;
    void joined(const LbmxEvent& event) override;
    void joinFail(const LbmxEvent& event) override;
    void alarm(const LbmxEvent& event) override;
    void time(const LbmxEvent& event) override;
    void almanacUpdate(const LbmxEvent& event) override;
    void txDone(const LbmxEvent& event) override;
    void downData(const LbmxEvent& event) override;

    void gnssScanDone(const LbmxEvent& event) override;
    void gnssScanCancelled(const LbmxEvent& event) override;
    void gnssTerminated(const LbmxEvent& event) override;
    void gnssErrorNoTime(const LbmxEvent& event) override;
    void gnssErrorAlmanacUpdate(const LbmxEvent& event) override;
    void gnssScanStopped(const LbmxEvent& event) override;
    void gnssErrorNoAidingPosition(const LbmxEvent& event) override;

};

void MyLbmxEventHandlers::reset(const LbmxEvent& event)
{

    if (LbmxEngine::setRegion(REGION) != SMTC_MODEM_RC_OK) abort();
    if (LbmxEngine::setOTAA(DEV_EUI, JOIN_EUI, APP_KEY) != SMTC_MODEM_RC_OK) abort();

    if (smtc_modem_dm_set_info_interval(SMTC_MODEM_DM_INFO_INTERVAL_IN_DAY, 1) != SMTC_MODEM_RC_OK) abort();
    {
        const uint8_t infoField = SMTC_MODEM_DM_FIELD_ALMANAC_STATUS;
        if (smtc_modem_dm_set_info_fields(&infoField, 1) != SMTC_MODEM_RC_OK) abort();
    }

    printf("Join the LoRaWAN network.\n");
    if (LbmxEngine::joinNetwork() != SMTC_MODEM_RC_OK) abort();

    // if((REGION == SMTC_MODEM_REGION_EU_868) || (REGION == SMTC_MODEM_REGION_RU_864)) //disable duty cycle limit
    // {
    //     smtc_modem_set_region_duty_cycle( false );
    // }
    state = StateType::Joining;
}

void MyLbmxEventHandlers::joined(const LbmxEvent& event)
{
    state = StateType::Joined;
    //Configure ADR, It is necessary to set up ADR,Tx useable payload must large than 51 bytes
    app_get_profile_list_by_region(REGION,adr_custom_list_region);
    if (smtc_modem_adr_set_profile(0, SMTC_MODEM_ADR_PROFILE_CUSTOM, adr_custom_list_region) != SMTC_MODEM_RC_OK) abort();              //adr_custom_list_region  CUSTOM_ADR  

    if (smtc_modem_time_set_sync_interval_s(TIME_SYNC_VALID_TIME / 3) != SMTC_MODEM_RC_OK) abort();     // keep call order
    if (smtc_modem_time_set_sync_invalid_delay_s(TIME_SYNC_VALID_TIME) != SMTC_MODEM_RC_OK) abort();    // keep call order

    printf("Start time sync.\n");
    if (smtc_modem_time_start_sync_service(0, SMTC_MODEM_TIME_ALC_SYNC) != SMTC_MODEM_RC_OK) abort();
    
    printf("Start the alarm event.\n");
    if (LbmxEngine::startAlarm(FIRST_UPLINK_DELAY) != SMTC_MODEM_RC_OK) abort();

}
void MyLbmxEventHandlers::joinFail(const LbmxEvent& event)
{
    state = StateType::Failed;
}
void MyLbmxEventHandlers::time(const LbmxEvent& event)
{
    if (event.event_data.time.status == SMTC_MODEM_EVENT_TIME_NOT_VALID) return;

    static bool first = true;
    if (first)
    {   
        if( is_first_time_sync == false )
        {
            is_first_time_sync = true;
        }
        printf("time sync ok:current time:%d\r\n",app_task_track_get_utc( ));
        // Configure transmissions
        if (smtc_modem_set_nb_trans(0, 1) != SMTC_MODEM_RC_OK) abort();
        if (smtc_modem_connection_timeout_set_thresholds(0, 0, 0) != SMTC_MODEM_RC_OK) abort();

        first = false;
    }
}
void MyLbmxEventHandlers::alarm(const LbmxEvent& event)
{

    static uint32_t counter = 0;
    if(app_task_lora_tx_engine())
    {
        ledOn(LED_BUILTIN);
    }
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
    uint32_t confirmed_count = 0;
    ledOff(LED_BUILTIN);
    if( event.event_data.txdone.status == SMTC_MODEM_EVENT_TXDONE_CONFIRMED )
    {
        app_lora_confirmed_count_increment();
    }
    uint32_t tick = smtc_modem_hal_get_time_in_ms( );
    confirmed_count = app_lora_get_confirmed_count();
    printf( "LoRa tx done at %u, %u, %u\r\n", tick, ++uplink_count, confirmed_count );    
}

void MyLbmxEventHandlers::downData(const LbmxEvent& event)
{
    uint8_t port;
    printf("Downlink received:\n");
    printf("  - LoRaWAN Fport = %d\n", event.event_data.downdata.fport);
    printf("  - Payload size  = %d\n", event.event_data.downdata.length);
    printf("  - RSSI          = %d dBm\n", event.event_data.downdata.rssi - 64);
    printf("  - SNR           = %d dB\n", event.event_data.downdata.snr / 4);

    if (event.event_data.downdata.length != 0)
    {
        port = event.event_data.downdata.fport;
        gnss_mw_handle_downlink(event.event_data.downdata.fport, event.event_data.downdata.data, event.event_data.downdata.length);
        if( port == app_lora_port )
        {
            app_lora_data_rx_size = event.event_data.downdata.length;
            memcpy( app_lora_data_rx_buffer, event.event_data.downdata.data, app_lora_data_rx_size );
            app_task_packet_downlink_decode( app_lora_data_rx_buffer, app_lora_data_rx_size );
            memset(app_lora_data_rx_buffer,0,app_lora_data_rx_size);
            app_lora_data_rx_size = 0;

        }
    }

}
 
void MyLbmxEventHandlers::gnssScanDone(const LbmxEvent& event)
{
    printf("----- GNSS - %s -----\n", event.getGnssEventString(GNSS_MW_EVENT_SCAN_DONE).c_str());

    gnss_mw_event_data_scan_done_t eventData;
    gnss_mw_get_event_data_scan_done(&eventData);

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
        if (smtc_modem_dm_request_single_uplink(&dmAlmanacStatus, 1) != SMTC_MODEM_RC_OK) abort();
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
void MyLbmxEventHandlers::gnssScanCancelled(const LbmxEvent& event)
{
    printf("----- GNSS - %s -----\n", event.getGnssEventString(GNSS_MW_EVENT_TERMINATED).c_str());
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
    if (smtc_modem_dm_request_single_uplink(&dmAlmanacStatus, 1) != SMTC_MODEM_RC_OK) abort();
    mw_gnss_event_state = GNSS_MW_EVENT_ERROR_ALMANAC_UPDATE;
}
void MyLbmxEventHandlers::gnssErrorNoAidingPosition(const LbmxEvent& event)
{
    printf("----- GNSS - %s -----\n", event.getGnssEventString(GNSS_MW_EVENT_ERROR_ALMANAC_UPDATE).c_str());

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
    default_param_load();
    init_current_lorawan_param();
    delay(1000);
    sensor_init_detect();

    app_ble_adv_init();

    printf("\n---------- STARTUP ----------\n");


    // custom_lora_adr_compute(0,6,adr_custom_list_region);

    lbmWm1110.attachGnssPrescan([](void* context){ digitalWrite(PIN_GNSS_LNA, HIGH); });
    lbmWm1110.attachGnssPostscan([](void* context){ digitalWrite(PIN_GNSS_LNA, LOW); });

    lbmWm1110.begin();

    app_gps_scan_init();
    user_button_init();

    track_scan_type_set(TRACKER_SCAN_GPS);

    // /* Initialize GNSS middleware */
    gnss_mw_init( lbmWm1110.getRadio(), stack_id );
    gnss_mw_custom_enable_copy_send_buffer();
    gnss_mw_set_constellations( GNSS_MW_CONSTELLATION_GPS_BEIDOU );

    // /* Set user defined assistance position */
    gnss_mw_set_user_aiding_position( app_task_gnss_aiding_position_latitude, app_task_gnss_aiding_position_longitude );
    
    LbmxEngine::begin(lbmWm1110.getRadio(), ModemEventHandler);
    LbmxEngine::printVersions(lbmWm1110.getRadio());

}

void loop()
{
    static uint32_t now_time = 0;
    static uint32_t start_scan_time = 0;  
    static uint32_t start_sensor_read_time = 0;   

    static uint16_t gnss_group_id_backup = track_gnss_group_id;
    bool result = false;  

    uint32_t sleepTime = LbmxEngine::doWork();

    if(is_first_time_sync == true)
    {
        if(sleepTime > 300)
        {  
            now_time = smtc_modem_hal_get_time_in_ms( );
            if(now_time - start_scan_time > position_period ||(start_scan_time == 0)||(button_trig_position == 1)||(move_trig_position == 1))
            {
                if(gps_scan_status != 2)
                {
                    if(app_gps_scan_start())
                    {
                        printf("start scan gnss\r\n");
                        gnss_scan_end = false;
                        start_scan_time = smtc_modem_hal_get_time_in_ms( );
                        consume_time = start_scan_time - now_time;
                    }
                    else
                    {
                        consume_time = smtc_modem_hal_get_time_in_ms() - now_time;                    
                    }
                }
            }
            if(gps_scan_status == 2)
            {
                result = app_gps_get_results( tracker_gps_scan_data, &tracker_gps_scan_len );
                if( result )
                {
                    //the consumption time is about 180ms
                    app_gps_display_results( );
                    app_gps_scan_stop( );
                    //Insert  position data to lora tx buffer   
                    app_task_track_scan_send();
                    button_trig_position = 0;
                    move_trig_position = 0;
                    printf("stop scan gnss\r\n");
                    if(gnss_group_id_backup != track_gnss_group_id)
                    {
                        printf("save track_gnss_group_id\r\n");
                        write_gnss_group_id_param();
                    }
                    consume_time = smtc_modem_hal_get_time_in_ms( ) - now_time; 
                }
                else if(((smtc_modem_hal_get_time_in_ms( ) - start_scan_time > gnss_scan_timeout)||(gnss_scan_end == true)))
                {
                    app_gps_scan_stop( );   
                    //Insert  position data to lora tx buffer
                    app_task_track_scan_send();
                    button_trig_position = 0;
                    move_trig_position = 0;
                    printf("stop scan gnss\r\n");
                    consume_time = smtc_modem_hal_get_time_in_ms( ) - now_time; 
                }                                                
            }
            sleepTime = sleepTime - consume_time; 
        }
        if(sleepTime > 50)
        { 
            now_time = smtc_modem_hal_get_time_in_ms();
            if(now_time - start_sensor_read_time > sensor_read_period ||(start_sensor_read_time == 0)||(button_trig_collect ==1)||(move_trig_collect == 1))
            {
                single_fact_sensor_data_get(lis3dhtr_sensor_type);                      //consume  1ms    
                single_fact_sensor_display_results(lis3dhtr_sensor_type);                 
                single_fact_sensor_data_get(sht4x_sensor_type);     //get temperture&humidity
                single_fact_sensor_display_results(sht4x_sensor_type);                
                factory_sensor_data_combined();
                //Insert all sensor data to lora tx buffer
                app_task_factory_sensor_data_send();
                button_trig_collect = 0;
                move_trig_collect = 0;
                start_sensor_read_time = smtc_modem_hal_get_time_in_ms( );
                consume_time = start_sensor_read_time - now_time; 
                sleepTime = sleepTime - consume_time;
            }
        }
    }  
    if(hal_ble_rec_data())
    {
        parse_cmd((char *)ble_rec_data_buf, ble_rec_data_len);         
        memset(ble_rec_data_buf,0,ble_rec_data_len);
        ble_rec_data_len = 0;
        ble_rec_done = false;        
    }
    if(ble_connect_status == 3)
    {
        smtc_modem_hal_reset_mcu();
    }
    delay(min(sleepTime, EXECUTION_PERIOD));
}

////////////////////////////////////////////////////////////////////////////////
