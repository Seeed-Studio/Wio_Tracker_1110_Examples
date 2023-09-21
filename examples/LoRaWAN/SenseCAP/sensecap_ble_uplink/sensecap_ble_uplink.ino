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

/* most important things:
// Please follow local regulations to set lorawan duty cycle limitations
// smtc_modem_set_region_duty_cycle()
//  
//  USER TODO:
//  1.Redefine parameters   =>      'DEV_EUI','JOIN_EUI','APP_KEY'
//  2.Comment code call     =>      'init_current_lorawan_param'
//  3.Modify parameters     =>      'position_period'
//
//  If the user has their own sensor
//  4.Realize Sensor Data Acquisition Put into 'user_data_buff',set 'user_data_len'  (it's must be 4bytes/group)
//  5.call  function                =>      'user_sensor_datas_set' 
//  6.call  function                =>      'app_task_user_sensor_data_send'
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


static constexpr uint32_t EXECUTION_PERIOD = 60000;    // [msec.]
static constexpr uint32_t BLE_SCAN_PERIOD = 300;    // [sec.]   //5 minutes minimum

////////////////////////////////////////////////////////////////////////////////
// Variables
uint32_t position_period = BLE_SCAN_PERIOD*1000;    // [msec.]
uint32_t ble_scan_timeout = 5000; // [msec.]
uint32_t consume_time = 0;

uint8_t DEV_EUI[8];
uint8_t JOIN_EUI[8];
uint8_t APP_KEY[16];

uint8_t user_data_buff[40];
uint8_t user_data_len = 0;

uint32_t sensor_read_period = 180*1000;   // [msec.]
uint32_t sound_sample_period = 10*1000;   // [msec.]
uint32_t ultrasonic_sample_period = 10*1000;   // [msec.]
uint16_t voc_sample_period = 10000;      // [msec.]

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
    void txDone(const LbmxEvent& event) override;
    void downData(const LbmxEvent& event) override;

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

    // if((REGION == SMTC_MODEM_REGION_EU_868) || (REGION == SMTC_MODEM_REGION_RU_864))
    // {
    //     smtc_modem_set_region_duty_cycle( false );
    // }
    state = StateType::Joining;
}

void MyLbmxEventHandlers::joined(const LbmxEvent& event)
{
    state = StateType::Joined;
    //Configure ADR, It is necessary to set up ADR,Tx useable payload must large than 51 bytes
    app_set_profile_list_by_region(REGION,adr_custom_list_region);
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
        printf("time sync ok:current time:%lu\r\n",app_task_track_get_utc( ));
        // Configure transmissions
        if (smtc_modem_set_nb_trans(0, 1) != SMTC_MODEM_RC_OK) abort();
        if (smtc_modem_connection_timeout_set_thresholds(0, 0, 0) != SMTC_MODEM_RC_OK) abort();

        first = false;
    }
}
void MyLbmxEventHandlers::alarm(const LbmxEvent& event)
{
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
        app_lora_confirmed_count_increment();
    }
    uint32_t tick = smtc_modem_hal_get_time_in_ms( );
    uint32_t confirmed_count = app_lora_get_confirmed_count();
    printf( "LoRa tx done at %lu, %lu, %lu\r\n", tick, ++uplink_count, confirmed_count );    
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
    tracker_scan_type_set(TRACKER_SCAN_BLE);

    printf("\n---------- STARTUP ----------\n");
    custom_lora_adr_compute(0,6,adr_custom_list_region);
    
    if(position_period<300000) position_period = 300000;        //Minimum 5 minutes
    lbmWm1110.begin();
    LbmxEngine::begin(lbmWm1110.getRadio(), ModemEventHandler);

    LbmxEngine::printVersions(lbmWm1110.getRadio());

}

void loop()
{
    static uint32_t now_time = 0;
	static uint32_t start_scan_time = 0;  
    static uint32_t start_sensor_read_time = 0;  
    static uint32_t start_voc_read_time = 0; 
    static uint32_t start_sound_read_time = 0; 
    static uint32_t start_ultrasonic_read_time = 0; 

    bool result = false;  
    uint32_t sleepTime = LbmxEngine::doWork();

    now_time = smtc_modem_hal_get_time_in_ms( );

    if(sleepTime > 300)
    {
        //temperture & humidity & voc
        if(now_time - start_voc_read_time > voc_sample_period ||(start_voc_read_time == 0))
        {
            //the consumption time is about 260ms
            single_fact_sensor_data_get(sht4x_sensor_type);     //get temperture&humidity for SGP internal compensation
            single_fact_sensor_display_results(sht4x_sensor_type);
            single_fact_sensor_data_get(sgp41_sensor_type);
            single_fact_sensor_display_results(sgp41_sensor_type);
            start_voc_read_time = smtc_modem_hal_get_time_in_ms( );
            consume_time = start_voc_read_time - now_time;
            sleepTime = sleepTime - consume_time;
        }
    }

    if(is_first_time_sync == true)
    {
        if(sleepTime > 300)
        {
            now_time = smtc_modem_hal_get_time_in_ms( );
            if(now_time - start_scan_time > position_period ||(start_scan_time == 0))
            {
                printf("start scan ibeacon\r\n");
                app_ble_scan_start();
                start_scan_time = smtc_modem_hal_get_time_in_ms( );
                consume_time = start_scan_time - now_time; 
            }

            if(smtc_modem_hal_get_time_in_ms( ) - start_scan_time > ble_scan_timeout &&(ble_scan_status == 2))
            {
                app_ble_scan_stop( );
                printf("stop scan ibeacon\r\n");
                result = app_ble_get_results( tracker_ble_scan_data, &tracker_ble_scan_len );
                if( result )
                {
                    app_ble_display_results( );
                }
                //Insert  position data to lora tx buffer
                app_task_track_scan_send();         //position data
                consume_time = smtc_modem_hal_get_time_in_ms( ) - now_time; 
            }
            sleepTime = sleepTime - consume_time; 
        }
        if(sleepTime > 1100)
        {
            now_time = smtc_modem_hal_get_time_in_ms();
            if(now_time - start_sensor_read_time > sensor_read_period ||(start_sensor_read_time == 0))
            {
                single_fact_sensor_data_get(lis3dhtr_sensor_type);                      //consume  1ms     if reinitialize => 602ms
                single_fact_sensor_data_get(dps310_sensor_type);                        //consume  219ms   if reinitialize => 335ms 
                single_fact_sensor_data_get(si1151_sensor_type);                        //consume  4ms     if reinitialize => 98ms
                factory_sensor_data_combined();
                app_sensor_data_display_results();
                //Insert all sensor data to lora tx buffer
                app_task_factory_sensor_data_send();
                start_sensor_read_time = smtc_modem_hal_get_time_in_ms( );
                consume_time = start_sensor_read_time - now_time; 
                sleepTime = sleepTime - consume_time;
            }
        }
    }
    if(sleepTime > 50)
    {
        now_time = smtc_modem_hal_get_time_in_ms();
        if(now_time - start_sound_read_time > sound_sample_period ||(start_sound_read_time == 0))
        {
            single_fact_sensor_data_get(sound_sensor_type);                 //30ms
            single_fact_sensor_display_results(sound_sensor_type);
            start_sound_read_time = smtc_modem_hal_get_time_in_ms( );
            consume_time = start_sound_read_time - now_time;            
            sleepTime = sleepTime - consume_time;
        }                          
    }        
    if(sleepTime > 50)
    {
        now_time = smtc_modem_hal_get_time_in_ms();
        if(now_time - start_ultrasonic_read_time > ultrasonic_sample_period ||(start_ultrasonic_read_time == 0))
        {
            single_fact_sensor_data_get(ultrasonic_sensor_type);      //if connected it will be 3ms,else 40ms timeout 
            single_fact_sensor_display_results(ultrasonic_sensor_type);
            if(ultrasonic_distance_cm < 10)
            {
                if(!relay_status_on())
                {
                    relay_status_control(true); 
                }
            }
            else if(relay_status_on())
            {
                relay_status_control(false);     
            }  
            start_ultrasonic_read_time = smtc_modem_hal_get_time_in_ms( );
            consume_time = start_ultrasonic_read_time - now_time;             
            sleepTime = sleepTime - consume_time;
        }                          
    }
    delay(min(sleepTime, EXECUTION_PERIOD));
}

////////////////////////////////////////////////////////////////////////////////
