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


static constexpr uint32_t WIFI_SCAN_PERIOD = 30;    // [sec.]

static constexpr uint32_t EXECUTION_PERIOD = 50;    // [msec.]

////////////////////////////////////////////////////////////////////////////////
// Variables

static LbmWm1110& lbmWm1110 = LbmWm1110::getInstance();

////////////////////////////////////////////////////////////////////////////////
// MyLbmxEventHandlers

class MyLbmxEventHandlers : public LbmxEventHandlers
{
protected:
    void wifiScanDone(const LbmxEvent& event) override;
    void wifiTerminated(const LbmxEvent& event) override;
    void wifiScanStopped(const LbmxEvent& event) override;

};

void MyLbmxEventHandlers::wifiScanDone(const LbmxEvent& event)
{
    printf("----- Wi-Fi - %s -----\n", event.getWifiEventString(WIFI_MW_EVENT_SCAN_DONE).c_str());

    static wifi_mw_event_data_scan_done_t wifiResults;
    wifi_mw_get_event_data_scan_done(&wifiResults);
    wifi_mw_display_results(&wifiResults);
}

void MyLbmxEventHandlers::wifiTerminated(const LbmxEvent& event)
{
    printf("----- Wi-Fi - %s -----\n", event.getWifiEventString(WIFI_MW_EVENT_TERMINATED).c_str());

    wifi_mw_event_data_terminated_t eventData;
    wifi_mw_get_event_data_terminated(&eventData);
    printf("TERMINATED info:\n");
    printf("-- number of scans sent: %u\n", eventData.nb_scans_sent);
}

void MyLbmxEventHandlers::wifiScanStopped(const LbmxEvent& event)
{
    if (wifi_mw_scan_start(WIFI_SCAN_PERIOD) != MW_RC_OK) abort();
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

    lbmWm1110.begin();
    LbmxEngine::begin(lbmWm1110.getRadio(), ModemEventHandler);

    LbmxEngine::printVersions(lbmWm1110.getRadio());

    if (wifi_mw_init(lbmWm1110.getRadio(), 0) != MW_RC_OK) abort();
    wifi_mw_send_bypass(true);
    if (wifi_mw_scan_start(0) != MW_RC_OK) abort();
}

extern "C" void loop()
{
    ledOn(LED_BUILTIN);
    const uint32_t sleepTime = LbmxEngine::doWork();
    ledOff(LED_BUILTIN);

    delay(min(sleepTime, EXECUTION_PERIOD));
}

////////////////////////////////////////////////////////////////////////////////
