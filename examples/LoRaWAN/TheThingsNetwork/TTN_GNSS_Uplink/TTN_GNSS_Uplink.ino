/*
 * TTN_GNSS_Uplink.ino
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

/* tips:
*   1.If you want to use LoRa Cloud™ Locator, refer to this(https://wiki.seeedstudio.com/connect_wio_tracker_to_locator/)
*       ①Login to LoRa Cloud™ Locator web(https://locator.loracloud.com/).
*       ②Configure your gateway(Eui etc)
*       ③Configure your device code(devEui,joinEui,nwkKey,region etc)
*       ④Replace in code devEui, joinEui, nwkKey, modem_region variable
*       ⑤Compile and download  
*
*   2.If you want to use Node-red
*       ①see https://lora-developers.semtech.com/build/software/lora-basics/lora-basics-for-end-nodes/developer-walk-through/?url=application_server.html
*       ②Configure your gateway in ttn or helium (Eui etc)
*       ③Configure your device code ttn or helium (devEui,joinEui,nwkKey,region etc)
*       ④Replace in code devEui, joinEui, nwkKey, modem_region variable
*       ⑤Compile and download 
*
*/

// Custom join network code
uint8_t devEui[SMTC_MODEM_EUI_LENGTH] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t joinEui[SMTC_MODEM_EUI_LENGTH] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t nwkKey[SMTC_MODEM_KEY_LENGTH] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// Custom region
smtc_modem_region_t modem_region = SMTC_MODEM_REGION_EU_868;

// Set a execution period
static constexpr uint32_t EXECUTION_PERIOD = 50;    // [msec.]

// Instance
static WM1110_Geolocation& wm1110_geolocation = WM1110_Geolocation::getInstance();

////////////////////////////////////////////////////////////////////////////////
// setup and loop

void setup()
{
    // 
    delay(1000);

    // Set the location mode to GNSS and uplink the data to other platform
    wm1110_geolocation.begin(Track_Scan_Gps,false);

    wm1110_geolocation.setCustomJoinNetworkCode(devEui,joinEui,nwkKey); // Set custom join network code 
    wm1110_geolocation.setCustomRegion(modem_region); // Set custom region 
    wm1110_geolocation.setCustomTrackPeriod(3); // Set custom period for positioning

    //Start running
    wm1110_geolocation.run();
}

void loop()
{
    // Run process 
    // sleepTime is the desired sleep time for LoRaWAN's next task
    uint32_t sleepTime = wm1110_geolocation.trackProcess();

    //delay
    delay(min(sleepTime, EXECUTION_PERIOD));
}

////////////////////////////////////////////////////////////////////////////////
