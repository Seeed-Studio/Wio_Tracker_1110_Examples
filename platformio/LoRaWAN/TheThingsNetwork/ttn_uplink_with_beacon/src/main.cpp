/*
 * main.cpp
 * Copyright (C) 2023 Seeed K.K.
 * MIT License
 */

////////////////////////////////////////////////////////////////////////////////
// Includes

#include <Arduino.h>
#include <bluefruit.h>

#include <LbmWm1110.hpp>
#include <Lbmx.hpp>

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

static const uint8_t DEV_EUI[8]  = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static const uint8_t JOIN_EUI[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static const uint8_t APP_KEY[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static constexpr smtc_modem_region_t REGION = SMTC_MODEM_REGION_AS_923_GRP1;

static constexpr uint16_t MANUFACTURER_ID = 0x0059; // Nordic Semiconductor ASA
static const uint8_t BEACON_UUID[16] =
{
    0x01, 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78,
    0x89, 0x9a, 0xab, 0xbc, 0xcd, 0xde, 0xef, 0xf0
};

static constexpr uint32_t EXECUTION_PERIOD = 50;    // [msec.]

////////////////////////////////////////////////////////////////////////////////
// Variables

static LbmWm1110& lbmWm1110 = LbmWm1110::getInstance();
static StateType state = StateType::Startup;

static BLEBeacon beacon(BEACON_UUID);

////////////////////////////////////////////////////////////////////////////////
// MyLbmxEventHandlers

class MyLbmxEventHandlers : public LbmxEventHandlers
{
protected:
    void reset(const LbmxEvent& event) override;
    void joined(const LbmxEvent& event) override;
    void joinFail(const LbmxEvent& event) override;

};

void MyLbmxEventHandlers::reset(const LbmxEvent& event)
{
    if (LbmxEngine::setRegion(REGION) != SMTC_MODEM_RC_OK) abort();
    if (LbmxEngine::setOTAA(DEV_EUI, JOIN_EUI, APP_KEY) != SMTC_MODEM_RC_OK) abort();

    printf("Join the LoRaWAN network.\n");
    if (LbmxEngine::joinNetwork() != SMTC_MODEM_RC_OK) abort();

    state = StateType::Joining;
}

void MyLbmxEventHandlers::joined(const LbmxEvent& event)
{
    state = StateType::Joined;
}

void MyLbmxEventHandlers::joinFail(const LbmxEvent& event)
{
    state = StateType::Failed;
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

    ////////////////////////////////////////
    // Beacon

    if (!Bluefruit.begin()) abort();
    Bluefruit.autoConnLed(false);
    Bluefruit.setTxPower(0);
    Bluefruit.setName("Wio Tracker 1110");
    Bluefruit.ScanResponse.addName();

    beacon.setManufacturer(MANUFACTURER_ID);

    Bluefruit.Advertising.setBeacon(beacon);
    Bluefruit.Advertising.restartOnDisconnect(true);
    Bluefruit.Advertising.setInterval(160, 160);    // in unit of 0.625 ms
    Bluefruit.Advertising.setFastTimeout(30);       // number of seconds in fast mode

    Bluefruit.Advertising.start(0);

    ////////////////////////////////////////
    // LBM

    lbmWm1110.begin();
    LbmxEngine::begin(lbmWm1110.getRadio(), ModemEventHandler);

    LbmxEngine::printVersions(lbmWm1110.getRadio());
}

extern "C" void loop()
{
    switch (state)
    {
    case StateType::Startup:
        ledOff(LED_BUILTIN);
        break;
    case StateType::Joining:
        if (millis() % 1000 < 200) ledOn(LED_BUILTIN); else ledOff(LED_BUILTIN);
        break;
    case StateType::Joined:
        ledOn(LED_BUILTIN);
        break;
    case StateType::Failed:
        if (millis() % 400 < 200) ledOn(LED_BUILTIN); else ledOff(LED_BUILTIN);
        break;
    }

    const uint32_t sleepTime = LbmxEngine::doWork();

    delay(min(sleepTime, EXECUTION_PERIOD));
}

////////////////////////////////////////////////////////////////////////////////
