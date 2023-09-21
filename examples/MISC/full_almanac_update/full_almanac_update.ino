/*
 * full_almanac_update.ino
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
// Please follow local regulations to set lorawan duty cycle limitations => smtc_modem_set_region_duty_cycle()
// 
//  Make sure the 'sleepTime' is greater than the time required to run the code.Otherwise, LoRaWAN will run incorrectly
//
//  USER TODO:
//  1.update almanac.h (use LBM_packet/src/internal/get_full_almanac.py)
//  
//  
//
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

static constexpr uint32_t EXECUTION_PERIOD = 60000;    // [msec.]

////////////////////////////////////////////////////////////////////////////////
// Variables
uint32_t position_period = 300*1000;   // [msec.]
uint32_t sensor_read_period = 180*1000;   // [msec.]

uint8_t DEV_EUI[8];
uint8_t JOIN_EUI[8];
uint8_t APP_KEY[16];

static  smtc_modem_region_t REGION = SMTC_MODEM_REGION_EU_868;

static LbmWm1110& lbmWm1110 = LbmWm1110::getInstance();
static StateType state = StateType::Startup;

////////////////////////////////////////////////////////////////////////////////
void print_current_lorawan_param(void)
{
    printf("DevEui:\r\n");
    for(uint8_t u8i = 0;u8i < 8; u8i++)
    {
        printf("%02x ",DEV_EUI[u8i]);
    }
    printf("\r\nJoinEui:\r\n");
    for(uint8_t u8i = 0;u8i < 8; u8i++)
    {
        printf("%02x ",JOIN_EUI[u8i]);
    }
    printf("\r\nAppKey:\r\n");
    for(uint8_t u8i = 0;u8i < 16; u8i++)
    {
        printf("%02x ",APP_KEY[u8i]);
    }
    printf("\r\n");    

    position_period = app_append_param.position_interval*60*1000;
    sensor_read_period = app_append_param.sample_interval*60*1000;

    printf("position_period:%umin\r\n",app_append_param.position_interval);
    printf("sensor_read_period:%umin\r\n",app_append_param.sample_interval);

}
void init_current_lorawan_param(void)
{
    memcpy(DEV_EUI,app_param.lora_info.DevEui,8);
    memcpy(JOIN_EUI,app_param.lora_info.JoinEui,8);
    memcpy(APP_KEY,app_param.lora_info.AppKey,16);

    REGION = sensecap_lorawan_region();	    

    print_current_lorawan_param();
}



// MyLbmxEventHandlers
class MyLbmxEventHandlers : public LbmxEventHandlers
{
protected:
    void reset(const LbmxEvent& event) override;
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

    printf("\n---------- STARTUP ----------\n");
    delay(3000);
    default_param_load();
    init_current_lorawan_param();

    lbmWm1110.begin();

    LbmxEngine::begin(lbmWm1110.getRadio(), ModemEventHandler);
    LbmxEngine::printVersions(lbmWm1110.getRadio());


    uint32_t almanac_date_in_lr1110 = app_task_radio_get_almanac_date();
    uint32_t almanac_date_in_buffer = get_almanac_date_from_buffer();
    printf("almanac_date lr1110:%u,buffer:%u\r\n",almanac_date_in_lr1110,almanac_date_in_buffer);

    app_task_full_almanac_update();  

    uint32_t almanac_date = app_task_radio_get_almanac_date();
    printf("almanac_date:%u\r\n",almanac_date);
}

void loop()
{
    uint32_t sleepTime = LbmxEngine::doWork();

    delay(min(sleepTime, EXECUTION_PERIOD));
}

////////////////////////////////////////////////////////////////////////////////
