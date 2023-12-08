/*
 * Default_Firmware.ino
 * Copyright (C) 2023 Seeed K.K.
 * MIT License
 */

////////////////////////////////////////////////////////////////////////////////
// Includes

#include <Arduino.h>

#include <LbmWm1110.hpp>
#include <Lbmx.hpp>

#include <Lbm_Modem_Common.hpp>
#include <WM1110_Geolocation.hpp>
#include <WM1110_BLE.hpp>
#include <WM1110_Storage.hpp>
#include <WM1110_At_Config.hpp>
#include <Tracker_Peripheral.hpp>

// Set a execution period
static constexpr uint32_t EXECUTION_PERIOD = 50;    // [msec.]

// Instance
static WM1110_Geolocation& wm1110_geolocation = WM1110_Geolocation::getInstance();

// Track 
uint32_t track_timeout = 2*60*1000;
uint32_t consume_time = 0;
uint32_t track_period_time = 0;  

// Sensor measurement
uint32_t start_sensor_read_time = 0; 
uint32_t sensor_read_period = 0; 

// Button interrupt
bool button_press_flag = false;
bool button_trig_track = false;
bool button_trig_collect = false;

// 3-axis interrupt
bool shock_flag = false;
bool shock_trig_track = false;
bool shock_trig_collect = false;

// Packed buf & size
uint8_t sensor_data_buf[64] = {0};
uint8_t sensor_data_size = 0;

// Sensor data
float x; 
float y; 
float z;
float temperature; 
float humidity;

// Receice cmd buf & size
uint8_t cmd_data_buf[244] = {0};
uint8_t cmd_data_size = 0;

// Get button and vibration actions to trigger positioning
void trigger_track_action(void)
{
    //button action detect
    tracker_peripheral.getUserButtonIrqStatus(&button_press_flag);
    if(button_press_flag)
    {
        printf("Button press down\r\n");
        if((button_trig_track == false) && (button_trig_collect == false))
        {
            button_trig_track = true;
            button_trig_collect = true;
            wm1110_geolocation.setEventStateAll(TRACKER_STATE_BIT0_SOS);    // Set event state
            tracker_peripheral.setSensorEventStatus(TRACKER_STATE_BIT0_SOS); 

        }
        tracker_peripheral.clearUserButtonFlag();
    }

    // Vibration action detect
    tracker_peripheral.getLIS3DHTRIrqStatus(&shock_flag);
    if(shock_flag)
    {
        printf("Shock trigger\r\n");
        if((shock_trig_track == false) && (shock_trig_track == false))
        {
            shock_trig_track = true;
            shock_trig_collect = true;
            wm1110_geolocation.setEventStateAll(TRACKER_STATE_BIT5_DEV_SHOCK);   // Set event state
            tracker_peripheral.setSensorEventStatus(TRACKER_STATE_BIT5_DEV_SHOCK);
        }
        tracker_peripheral.clearShockFlag();
    }
}


////////////////////////////////////////////////////////////////////////////////
// setup and loop
void setup()
{
    // Initializes the storage area
    wm1110_storage.begin();
    wm1110_storage.loadBootConfigParameters(); // Load all parameters (WM1110_Param_Var.h)

    delay(1000);

    wm1110_ble.begin(); // Init BLE
    wm1110_ble.setName(); // Set the  Bluetooth broadcast name

    // Set broadcast parameters
    // true: central,  false:peripheral   empty:both
    wm1110_ble.setStartParameters();

    // Start broadcast
    wm1110_ble.startAdv();

    // Initializes the detected IIC peripheral sensors(include LIS3DHTR,SHT4x,Si1151,SGP40,DPS310)    
    tracker_peripheral.begin();
    tracker_peripheral.setUserButton(); // Set user button detection

    // Set the location mode to GNSS and uplink the data to SenseCAP platform
    wm1110_geolocation.begin(Track_Scan_Gps,true);

    // Initialize command parsing
    wm1110_at_config.begin();

    sensor_read_period = wm1110_geolocation.getSensorMeasurementPeriod();    // Get period for reading sensor data
    sensor_read_period = sensor_read_period*60*1000;  // Convert to ms

    track_period_time = wm1110_geolocation.getTrackPeriod();    // Get period for positioning
    track_period_time = track_period_time*60*1000;  // Convert to ms  

    track_timeout = wm1110_geolocation.getTrackTimeout();
    track_timeout = track_timeout *1000;  // Convert to ms

    // Start running    
    wm1110_geolocation.run();
}

void loop()
{
    static uint32_t now_time = 0;
    static uint32_t start_scan_time = 0;   

    bool result = false;    

    // Run process 
    // sleepTime is the desired sleep time for LoRaWAN's next task (Periodic positioning is not included)    
    uint32_t sleepTime = wm1110_geolocation.lbmxProcess(); 

    // Light action for Join 
    wm1110_geolocation.modemLedActionProcess();

    if(wm1110_geolocation.time_sync_flag== true) // The device has been synchronized time from the LNS
    {
        trigger_track_action();

        // track
        if(sleepTime > 300) // Free time
        {  
            now_time = smtc_modem_hal_get_time_in_ms( );
            switch(wm1110_geolocation.tracker_scan_status) // Tracker scan status
            {
                case Track_None:
                case Track_Start:
                    if(now_time - start_scan_time > track_period_time ||(start_scan_time == 0)||button_trig_track ||shock_trig_track)
                    {
                        if(wm1110_geolocation.startTrackerScan()) // Start positioning
                        {
                            printf("Start tracker scan\r\n");
                            start_scan_time = smtc_modem_hal_get_time_in_ms( );
                            consume_time = start_scan_time - now_time;
                        }
                        else
                        {
                            consume_time = smtc_modem_hal_get_time_in_ms() - now_time;                    
                        } 
                    }
                    break;
                case Track_Scaning:
                    if(smtc_modem_hal_get_time_in_ms( ) - start_scan_time > track_timeout)
                    {
                        wm1110_geolocation.stopTrackerScan(); // Timeout,stop positioning    
                        result = wm1110_geolocation.getTrackResults( ); // Get results
                        if(result)
                        {
                            wm1110_geolocation.displayTrackRawDatas(); // Dispaly raw datas
                        } 
                    }
                    break;
                case Track_End:
                    result = wm1110_geolocation.getTrackResults( ); // End of position,get results
                    if(result)
                    {
                        wm1110_geolocation.displayTrackRawDatas();
                    }
                    wm1110_geolocation.stopTrackerScan(); // Stop positioning 
                    break;
                case Track_Stop:
                    printf("Stop tracker scan\r\n");
                    //Insert  position data to lora tx buffer 
                    wm1110_geolocation.insertTrackResultsIntoQueue();
                    consume_time = smtc_modem_hal_get_time_in_ms( ) - now_time; 
                    wm1110_geolocation.tracker_scan_status = Track_None;
                    button_trig_track = false;
                    shock_trig_track = false;
                    break;
                default :
                    break;
            }
            sleepTime = sleepTime - consume_time; 
        }
        // sensor 
        if(sleepTime > 500) // Free time
        { 
            now_time = smtc_modem_hal_get_time_in_ms();
            if(now_time - start_sensor_read_time > sensor_read_period ||(start_sensor_read_time == 0) ||button_trig_collect ||shock_trig_collect)
            {
                printf("Reading sensor data...\r\n");
                tracker_peripheral.measureLIS3DHTRDatas(&x,&y,&z); // Get the 3-axis data
                tracker_peripheral.measureSHT4xDatas(&temperature,&humidity); // Get the Temperture & humidity data
                // Pack datas      
                tracker_peripheral.packUplinkSensorDatas();
                // Display sensor raw datas 
                tracker_peripheral.displaySensorDatas();
                // Get packed datas
                tracker_peripheral.getUplinkSensorDatas( sensor_data_buf, &sensor_data_size );

                // Insert all sensor data to lora tx buffer
                wm1110_geolocation.insertIntoTxQueue(sensor_data_buf,sensor_data_size);
                start_sensor_read_time = smtc_modem_hal_get_time_in_ms( );
                consume_time = start_sensor_read_time - now_time; 
                sleepTime = sleepTime - consume_time;
                button_trig_collect = false;
                shock_trig_collect = false;
            }
        }
    }  
    if(wm1110_ble.getBleRecData(cmd_data_buf,&cmd_data_size))    // Communicate commands with APP
    {
        cmd_parse_type = 1;
        wm1110_at_config.parseCmd((char *)cmd_data_buf,cmd_data_size);
        memset(cmd_data_buf,0,cmd_data_size);
        cmd_data_size = 0;
        cmd_parse_type = 0;
    }
    //The Bluetooth connection is disconnected. Restart the device to make the configuration take effect 
    if(wm1110_ble.getBleStatus() == BleRunState::StateDisconnect)
    {
        smtc_modem_hal_reset_mcu();
    }

    delay(min(sleepTime, EXECUTION_PERIOD));
}

////////////////////////////////////////////////////////////////////////////////
