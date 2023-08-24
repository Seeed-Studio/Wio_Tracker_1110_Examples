/*
 * main.cpp
 * Copyright (C) 2023 Seeed K.K.
 * MIT License
 */

////////////////////////////////////////////////////////////////////////////////
// Includes

#include <Arduino.h>

#include <LbmWm1110.hpp>
#include <Lbmx.hpp>

////////////////////////////////////////////////////////////////////////////////
// Constants

static const uint8_t DEV_EUI[8]  = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static const uint8_t JOIN_EUI[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static const uint8_t APP_KEY[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static constexpr smtc_modem_region_t REGION = SMTC_MODEM_REGION_AS_923_GRP1;
static const uint8_t CUSTOM_ADR[SMTC_MODEM_CUSTOM_ADR_DATA_LENGTH] = { 5, 5, 5, 5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 3, 3 };
static constexpr uint8_t TRANS_NUMBER = 1;

static constexpr uint32_t TIME_SYNC_VALID_TIME = 60 * 60 * 24;  // [sec.]

static constexpr uint32_t GNSS_SCAN_PERIOD = 30;    // [sec.]
static constexpr gnss_mw_mode_t GNSS_SCAN_MODE = GNSS_MW_MODE_MOBILE;

static constexpr uint32_t EXECUTION_PERIOD = 50;    // [msec.]

////////////////////////////////////////////////////////////////////////////////
// Variables

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
    void time(const LbmxEvent& event) override;
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
    if (LbmxEngine::setOTAA(DEV_EUI, JOIN_EUI, APP_KEY) != SMTC_MODEM_RC_OK) abort();

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
}

void MyLbmxEventHandlers::time(const LbmxEvent& event)
{
    if (event.event_data.time.status == SMTC_MODEM_EVENT_TIME_NOT_VALID) return;

    static bool first = true;
    if (first)
    {
        // Configure ADR and transmissions
        if (smtc_modem_adr_set_profile(0, SMTC_MODEM_ADR_PROFILE_CUSTOM, CUSTOM_ADR) != SMTC_MODEM_RC_OK) abort();
        if (smtc_modem_set_nb_trans(0, TRANS_NUMBER) != SMTC_MODEM_RC_OK) abort();
        if (smtc_modem_connection_timeout_set_thresholds(0, 0, 0) != SMTC_MODEM_RC_OK) abort();

        // Initialize GNSS middleware
        if (gnss_mw_init(lbmWm1110.getRadio(), 0) != MW_RC_OK) abort();
        gnss_mw_set_constellations(GNSS_MW_CONSTELLATION_GPS_BEIDOU);

        printf("Start GNSS scan.\n");
        if (gnss_mw_scan_start(GNSS_SCAN_MODE, 0) != MW_RC_OK) abort();

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

    if (eventData.context.almanac_update_required)
    {
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
}

void MyLbmxEventHandlers::gnssErrorAlmanacUpdate(const LbmxEvent& event)
{
    printf("----- GNSS - %s -----\n", event.getGnssEventString(GNSS_MW_EVENT_ERROR_ALMANAC_UPDATE).c_str());

    if (LbmxDeviceManagement::requestUplink({ SMTC_MODEM_DM_FIELD_ALMANAC_STATUS }) != SMTC_MODEM_RC_OK) abort();
}

void MyLbmxEventHandlers::gnssScanStopped(const LbmxEvent& event)
{
    if (gnss_mw_scan_start(GNSS_SCAN_MODE, GNSS_SCAN_PERIOD) != MW_RC_OK) abort();
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

extern "C" void setup()
{
    delay(1000);
    printf("\n---------- STARTUP ----------\n");

    lbmWm1110.attachGnssPrescan([](void* context){ digitalWrite(PIN_GNSS_LNA, HIGH); });
    lbmWm1110.attachGnssPostscan([](void* context){ digitalWrite(PIN_GNSS_LNA, LOW); });
    lbmWm1110.begin();
    LbmxEngine::begin(lbmWm1110.getRadio(), ModemEventHandler);

    LbmxEngine::printVersions(lbmWm1110.getRadio());
}

extern "C" void loop()
{
    ledOn(LED_BUILTIN);
    const uint32_t sleepTime = LbmxEngine::doWork();
    ledOff(LED_BUILTIN);

    delay(min(sleepTime, EXECUTION_PERIOD));
}

////////////////////////////////////////////////////////////////////////////////
