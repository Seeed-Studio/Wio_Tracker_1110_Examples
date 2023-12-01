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



////////////////////////////////////////////////////////////////////////////////
// Constants

// // network code for loracloud
// see https://locator.loracloud.com/device-tracking

uint8_t devEui[SMTC_MODEM_EUI_LENGTH] = {0x00, 0x16, 0xC0, 0x01, 0xFF, 0xFE, 0x10, 0x31};
uint8_t joinEui[SMTC_MODEM_EUI_LENGTH] = {0x00, 0x16, 0xC0, 0x01, 0xFF, 0xFE, 0x00, 0x01};
uint8_t nwkKey[SMTC_MODEM_KEY_LENGTH] = {0x00, 0x16, 0xC0, 0x01, 0xFF, 0xFE, 0x10, 0x31, 0x00, 0x16, 0xC0, 0x01, 0xFF, 0xFE, 0x00, 0x01};

// network code for node-red 
// see https://lora-developers.semtech.com/build/software/lora-basics/lora-basics-for-end-nodes/developer-walk-through/?url=application_server.html

// uint8_t devEui[SMTC_MODEM_EUI_LENGTH] = {0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x06, 0x23, 0x14};
// uint8_t joinEui[SMTC_MODEM_EUI_LENGTH] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
// uint8_t nwkKey[SMTC_MODEM_KEY_LENGTH] = {0xCA, 0x45, 0x8D, 0xE6, 0xCD, 0xDC, 0xC4, 0x77, 0x45, 0xCE, 0x4A, 0x59, 0xF8, 0x75, 0xA2, 0x17};

smtc_modem_region_t modem_region = SMTC_MODEM_REGION_EU_868;

static constexpr uint32_t EXECUTION_PERIOD = 50;    // [msec.]
static WM1110_Geolocation& wm1110_geolocation = WM1110_Geolocation::getInstance();



////////////////////////////////////////////////////////////////////////////////
// setup and loop

void setup()
{
    // 
    delay(1000);
    
    wm1110_geolocation.begin(Track_Scan_Wifi,false);

    wm1110_geolocation.setCustomJoinNetworkCode(devEui,joinEui,nwkKey);
    wm1110_geolocation.setCustomRegion(modem_region);
    wm1110_geolocation.setCustomTrackPeriod(3);

    wm1110_geolocation.run();
}

void loop()
{

    uint32_t sleepTime = wm1110_geolocation.trackProcess();
    
    delay(min(sleepTime, EXECUTION_PERIOD));
}

////////////////////////////////////////////////////////////////////////////////
