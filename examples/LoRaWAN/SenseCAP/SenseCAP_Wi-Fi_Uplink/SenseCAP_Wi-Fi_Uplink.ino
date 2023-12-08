/*
 * SenseCAP_Wi-Fi_Uplink.ino
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

// Relay pin
static constexpr int relay_pin    = D2;

//Track 
uint32_t consume_time = 0;

//Sensor measurement
uint32_t sensor_read_period = 0; 
static uint32_t start_sensor_read_time = 0; 
static uint32_t start_voc_read_time = 0; 
static uint32_t start_sound_read_time = 0; 
static uint32_t start_ultrasonic_read_time = 0; 

uint32_t sound_sample_period = 10*1000;   // [msec.]
uint32_t ultrasonic_sample_period = 10*1000;   // [msec.]
uint16_t voc_sample_period = 10000;      // [msec.]

// Packed buf & size
uint8_t sensor_data_buf[64] = {0};
uint8_t sensor_data_size = 0;

//Sensor data
float x; 
float y; 
float z;
float temperature; 
float humidity;
int32_t tVOC_index;
float uv_value;  
uint16_t sound_value; 
uint16_t ultrasonic_distance_cm; 
uint32_t barometric_value;   
bool relay_switch_on = false;

// Relay
void relay_status_control( bool switch_on )
{
    if(switch_on)
    {
        digitalWrite(relay_pin, HIGH);   
        pinMode(relay_pin, OUTPUT); 
        relay_switch_on = true;
    }
    else
    {
        digitalWrite(relay_pin, LOW);   
        pinMode(relay_pin, OUTPUT); 
        relay_switch_on = false;        
    }
}

bool relay_status_on( void )
{
    return relay_switch_on;
}

////////////////////////////////////////////////////////////////////////////////
// setup and loop

void setup()
{
    // Initializes the storage area
    wm1110_storage.begin();
    wm1110_storage.loadBootConfigParameters(); // Load all parameters (WM1110_Param_Var.h)

    delay(1000);

    // Initializes the detected IIC peripheral sensors(include LIS3DHTR,SHT4x,Si1151,SGP40,DPS310)
    tracker_peripheral.begin();

    // Set the location mode to Wi-Fi and uplink the data to SenseCAP platform
    wm1110_geolocation.begin(Track_Scan_Wifi,true);
    wm1110_geolocation.setSensorMeasurementPeriod(3);    // Set custom period for reading sensor data 

    sensor_read_period = wm1110_geolocation.getSensorMeasurementPeriod();
    sensor_read_period = sensor_read_period*60*1000;  // Convert to ms

    //Start running
    wm1110_geolocation.run();
}


void loop()
{
    uint32_t now_time = 0;
    bool result = false; 

    // Run process 
    // sleepTime is the desired sleep time for LoRaWAN's next task
    uint32_t sleepTime = wm1110_geolocation.trackProcess();

    now_time = smtc_modem_hal_get_time_in_ms( );

    // The following sensors have different sampling periods, but all upload data during sensor_read_period
    if(sleepTime > 300) // Free time
    {
        //temperture & humidity & voc
        if(now_time - start_voc_read_time > voc_sample_period ||(start_voc_read_time == 0))
        {
            //the consumption time is about 260ms
            if(tracker_peripheral.measureSHT4xDatas(&temperature,&humidity)) // Get the Temperture & humidity data
            {
                printf("Temperature:%0.2f\r\nHumidity:%0.2f%%\r\n",temperature,humidity);
            }    
            // Get tVOC,(need to use temperature and humidity compensation and algorithms)
            if(tracker_peripheral.measureSGP41Datas(temperature,humidity,&tVOC_index))
            {
                printf("tVOC index:%d\r\n",tVOC_index); 
            }

            start_voc_read_time = smtc_modem_hal_get_time_in_ms( );
            consume_time = start_voc_read_time - now_time;
            sleepTime = sleepTime - consume_time;
        }
    }

    if(wm1110_geolocation.time_sync_flag== true)
    {
        if(sleepTime > 1100)
        {
            now_time = smtc_modem_hal_get_time_in_ms();
            if(now_time - start_sensor_read_time > sensor_read_period ||(start_sensor_read_time == 0))
            {
                // Get the 3-axis data
                if(tracker_peripheral.measureLIS3DHTRDatas(&x,&y,&z))
                {
                    printf("3-axis: x:%0.2f y:%0.2f z:%0.2f\r\n",x,y,z);
                }
                // Get the barometric data
                if(tracker_peripheral.measureDPS310(&barometric_value))
                {
                    printf("Atmospheric pressure:%lu\r\n",barometric_value);
                }
                // Get the ultraviolet data
                if(tracker_peripheral.measureSi1151Datas(&uv_value))
                {
                    printf("Ultraviolet illumination:%0.2f\r\n",uv_value);
                }
                // Pack datas
                tracker_peripheral.packUplinkSensorDatas();
                // Get packed datas                
                tracker_peripheral.getUplinkSensorDatas( sensor_data_buf, &sensor_data_size );   
                
                // Insert all sensor data to lora tx buffer
                wm1110_geolocation.insertIntoTxQueue(sensor_data_buf,sensor_data_size);                
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
            // Get sound adc value
            if(tracker_peripheral.measureSoundAdc(&sound_value))
            {
                printf("Sound analog:%d mV\r\n",sound_value);
            }

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
            // Get Ultrasonic distance datas 0~400cm,if out of range may be due to improper use
            if(tracker_peripheral.measureUltrasonicDistance(&ultrasonic_distance_cm))
            {
                printf("Ultrasonic distance:%d cm\r\n",ultrasonic_distance_cm);
            }

            // Control relay
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
    
    //delay
    delay(min(sleepTime, EXECUTION_PERIOD));
}

////////////////////////////////////////////////////////////////////////////////
