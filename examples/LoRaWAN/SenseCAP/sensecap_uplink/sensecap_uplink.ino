/*
 * sensecap_uplink.ino
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

static constexpr uint32_t FIRST_UPLINK_DELAY = 60;  // [sec.]
static constexpr uint32_t UPLINK_PERIOD = 30;       // [sec.]
static constexpr uint8_t UPLINK_FPORT = 5;

static constexpr uint32_t EXECUTION_PERIOD = 50;    // [msec.]

////////////////////////////////////////////////////////////////////////////////
// Variables

static LbmWm1110& lbmWm1110 = LbmWm1110::getInstance();
static StateType state = StateType::Startup;
uint8_t DEV_EUI[8];
uint8_t JOIN_EUI[8];
uint8_t APP_KEY[16];
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

    printf("Start the alarm event.\n");
    if (LbmxEngine::startAlarm(FIRST_UPLINK_DELAY) != SMTC_MODEM_RC_OK) abort();
}

void MyLbmxEventHandlers::joinFail(const LbmxEvent& event)
{
    state = StateType::Failed;
}

void MyLbmxEventHandlers::alarm(const LbmxEvent& event)
{
    printf("Send the uplink message.\n");
    static uint32_t counter = 0;
    if (LbmxEngine::requestUplink(UPLINK_FPORT, false, &counter, sizeof(counter)) != SMTC_MODEM_RC_OK) abort();
    ++counter;

    if (LbmxEngine::startAlarm(UPLINK_PERIOD) != SMTC_MODEM_RC_OK) abort();
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
    printf("\n---------- STARTUP ----------\n");

    lbmWm1110.begin();
    LbmxEngine::begin(lbmWm1110.getRadio(), ModemEventHandler);

    LbmxEngine::printVersions(lbmWm1110.getRadio());
}

void loop()
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
