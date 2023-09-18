#include <Arduino.h>

#include <LbmWm1110.hpp>
#include <Lbmx.hpp>

#include <Lbm_packet.hpp>


//taskhandle
static TaskHandle_t  LORAWAN_ENGINE_Handle;
static TaskHandle_t  LORAWAN_TX_Handle;

void app_lora_engine_task_wakeup( void );
void app_lora_tx_task_wakeup( void );

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

static constexpr uint8_t UPLINK_FPORT = 5;

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

};

void MyLbmxEventHandlers::reset(const LbmxEvent& event)
{
    if (LbmxEngine::setRegion(REGION) != SMTC_MODEM_RC_OK) abort();
    if (LbmxEngine::setOTAA(DEV_EUI, JOIN_EUI, APP_KEY) != SMTC_MODEM_RC_OK) abort();

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

    printf("Start the alarm event.\n");
    app_lora_tx_task_wakeup( );
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

//---------------Task---------------- ---------------
void app_lora_tx_task_wakeup( void )
{
    xTaskNotifyGive( LORAWAN_TX_Handle );
}
void app_Lora_engine_task_wakeup( void )
{
    xTaskNotifyGive( LORAWAN_ENGINE_Handle );
}

// LoraWan_Engine_Task
void LoraWan_Engine_Task(void *parameter) {
    while (true) 
    {
        uint32_t sleepTime = LbmxEngine::doWork();
        ( void )ulTaskNotifyTake( pdTRUE, sleepTime );
    }
}
// LoraWan_Tx_Task
void LoraWan_Tx_Task(void *parameter) {
    static uint32_t counter = 0;
    while (true) 
    {
        ( void )ulTaskNotifyTake( pdTRUE, portMAX_DELAY );
        while(1)
        {
            smtc_modem_status_mask_t modem_status;
            smtc_modem_get_status( 0, &modem_status );
            if(( modem_status & SMTC_MODEM_STATUS_JOINED ) == SMTC_MODEM_STATUS_JOINED)
            {
                printf("Send the uplink message.\n");
                LbmxEngine::requestUplink(UPLINK_FPORT, false, &counter, sizeof(counter));
                counter ++;
                app_Lora_engine_task_wakeup(  );
            }
            vTaskDelay(30000);
        }
    }
}
 
void setup() {

    delay(1000);
    printf("\n---------- STARTUP ----------\n");

    default_param_load();

    lbmWm1110.begin();
    LbmxEngine::begin(lbmWm1110.getRadio(), ModemEventHandler);

    LbmxEngine::printVersions(lbmWm1110.getRadio());
	// create task
	xTaskCreate(LoraWan_Engine_Task, "LoraWan_Engine_Task", 256*4, NULL, 1, &LORAWAN_ENGINE_Handle);

    xTaskCreate(LoraWan_Tx_Task, "LoraWan_Tx_Task", 256*2, NULL, 2, &LORAWAN_TX_Handle);

	vTaskStartScheduler();
}
 
void loop() {
	
}
