/*
 * sensecap_ble_uplink.ino
 * Copyright (C) 2023 Seeed K.K.
 * MIT License
 */

////////////////////////////////////////////////////////////////////////////////
// Includes

#include <Arduino.h>

#include <LbmWm1110.hpp>
#include <Lbmx.hpp>

#include <Lbm_packet.hpp>



/* most important thing:
// Please follow local regulations to set lorawan duty cycle limitations
// smtc_modem_set_region_duty_cycle()
//
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

static constexpr smtc_modem_region_t REGION = SMTC_MODEM_REGION_EU_868;

static constexpr uint32_t TIME_SYNC_VALID_TIME = 60 * 60 * 24;  // [sec.] 
static constexpr uint32_t FIRST_UPLINK_DELAY = 20;  // [sec.]
static constexpr uint32_t UPLINK_PERIOD = 15;       // [sec.]


static constexpr uint32_t EXECUTION_PERIOD = 50;    // [msec.]
static constexpr uint32_t BLE_SCAN_PERIOD = 120;    // [sec.]   //2 minutes minimum

////////////////////////////////////////////////////////////////////////////////
// Variables
uint32_t position_period = BLE_SCAN_PERIOD*1000;    // [msec.]
uint32_t ble_scan_timeout = 5000; // [msec.]

uint8_t DEV_EUI[8];
uint8_t JOIN_EUI[8];
uint8_t APP_KEY[16];



static LbmWm1110& lbmWm1110 = LbmWm1110::getInstance();
static StateType state = StateType::Startup;

////////////////////////////////////////////////////////////////////////////////
void init_current_lorawan_param(void)
{
    memcpy(DEV_EUI,app_param.lora_info.DevEui,8);
    memcpy(JOIN_EUI,app_param.lora_info.JoinEui,8);
    memcpy(APP_KEY,app_param.lora_info.AppKey,16);
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
    void txDone(const LbmxEvent& event);      
};
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
        if (smtc_modem_adr_set_profile(0, SMTC_MODEM_ADR_PROFILE_CUSTOM, adr_custom_list_region) != SMTC_MODEM_RC_OK) abort();              //adr_custom_list_region  CUSTOM_ADR
        if (smtc_modem_set_nb_trans(0, 1) != SMTC_MODEM_RC_OK) abort();
        if (smtc_modem_connection_timeout_set_thresholds(0, 0, 0) != SMTC_MODEM_RC_OK) abort();

        first = false;
    }
}
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

    if((REGION == SMTC_MODEM_REGION_EU_868) || (REGION == SMTC_MODEM_REGION_RU_864))
    {
        smtc_modem_set_region_duty_cycle( false );
    }

    state = StateType::Joining;
}

void MyLbmxEventHandlers::joined(const LbmxEvent& event)
{
    state = StateType::Joined;
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

    app_ble_scan_init();   

    printf("\n---------- STARTUP ----------\n");
    custom_lora_adr_compute(0,6,adr_custom_list_region);

    lbmWm1110.begin();
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
            if(now_time - start_scan_time > 120000 ||(start_scan_time == 0))
            {
                printf("start Scan ibeacon\r\n");
                app_ble_scan_start();
                start_scan_time = smtc_modem_hal_get_time_in_ms( );
            }
            // ledOff(LED_BUILTIN);
            if(smtc_modem_hal_get_time_in_ms( ) - start_scan_time > ble_scan_timeout &&(ble_scan_status == 2))
            {
                result = app_ble_get_results( tracker_ble_scan_data, &tracker_ble_scan_len );
                if( result )
                {
                    app_ble_display_results( );
                }
                sensor_datas_get();
                //send data to LoRaWAN
                app_task_track_scan_send();
                app_ble_scan_stop( );
                printf("stop Scan ibeacon\r\n");
            }
        }
        sleepTime = smtc_modem_hal_get_time_in_ms( )-now_time;
    }
    delay(min(sleepTime, EXECUTION_PERIOD));
}

////////////////////////////////////////////////////////////////////////////////
