/*
 * main.cpp
 * Copyright (C) 2024 Seeed K.K.
 * MIT License
 */

////////////////////////////////////////////////////////////////////////////////
// Includes

#include <Arduino.h>

#include <LbmWm1110.hpp>
#include <internal/Wm1110Hardware.hpp>
#include <lbm/smtc_modem_core/radio_drivers/lr11xx_driver/src/lr11xx_system.h>
#include <lbm/smtc_modem_core/radio_drivers/lr11xx_driver/src/lr11xx_bootloader.h>

#include "lr1110_transceiver_0307.h"
// #include "lr1110_transceiver_0308.h"
// #include "lr1110_transceiver_0401.h"

////////////////////////////////////////////////////////////////////////////////
// Constants

static const char VERSION[] = "1.0";

#define DLM "\r\n"

static constexpr uint32_t EXECUTION_PERIOD = 5000; // [msec.]

////////////////////////////////////////////////////////////////////////////////
// Variables

static Wm1110Hardware &wm1110Hw = Wm1110Hardware::getInstance();


////////////////////////////////////////////////////////////////////////////////
// Copy from SWTL001 (LR11xx Updater tool)
// https://github.com/Lora-net/SWTL001

////////////////////////////////////////
// https://github.com/Lora-net/SWTL001/blob/95c32ff18075a520ffa0eaf7e7ef85fd9d34acdc/application/inc/lr11xx_firmware_update.h

typedef enum
{
    LR1110_FIRMWARE_UPDATE_TO_TRX,
    LR1110_FIRMWARE_UPDATE_TO_MODEM,
    LR1120_FIRMWARE_UPDATE_TO_TRX,
    LR1121_FIRMWARE_UPDATE_TO_TRX,
} lr11xx_fw_update_t;

////////////////////////////////////////
// https://github.com/Lora-net/SWTL001/blob/95c32ff18075a520ffa0eaf7e7ef85fd9d34acdc/application/src/lr11xx_firmware_update.c

#define LR11XX_TYPE_PRODUCTION_MODE 0xDF

static bool lr11xx_is_chip_in_production_mode(uint8_t type)
{
    return (type == LR11XX_TYPE_PRODUCTION_MODE) ? true : false;
}

static bool lr11xx_is_fw_compatible_with_chip(lr11xx_fw_update_t update, uint16_t bootloader_version)
{
    if (((update == LR1110_FIRMWARE_UPDATE_TO_TRX) || (update == LR1110_FIRMWARE_UPDATE_TO_MODEM)) && (bootloader_version != 0x6500))
    {
        return false;
    }
    else if ((update == LR1120_FIRMWARE_UPDATE_TO_TRX) && (bootloader_version != 0x2000))
    {
        return false;
    }
    else if ((update == LR1121_FIRMWARE_UPDATE_TO_TRX) && (bootloader_version != 0x2100))
    {
        return false;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
static int lr111xx_info_get( void )
{
    lr11xx_system_version_t systemVersion;
    if (lr11xx_system_get_version(wm1110Hw.radio.ral.context, &systemVersion) != LR11XX_STATUS_OK)
    {
        Serial.printf("ERROR: Failed to get system version." DLM);
        return -1;
    }

    if (!lr11xx_is_chip_in_production_mode(systemVersion.type))
    {
        Serial.printf("Chip is in firmware mode." DLM);
        Serial.printf("Chip:" DLM);
        Serial.printf(" Hardware version = 0x%02X" DLM, systemVersion.hw);
        Serial.printf(" Type             = 0x%02X" DLM, systemVersion.type);
        Serial.printf(" Firmware version = 0x%04X" DLM, systemVersion.fw);

        // lr11xx_system_uid_t uid;
        // lr11xx_system_read_uid(wm1110Hw.radio.ral.context, uid);
        // Serial.printf(" UID = 0x%02X%02X%02X%02X%02X%02X%02X%02X" DLM, uid[0], uid[1], uid[2], uid[3], uid[4], uid[5], uid[6], uid[7]);
    }
    else
    {
        Serial.printf("Chip is in bootloader mode." DLM);
        Serial.printf("Chip:" DLM);
        Serial.printf(" Hardware version   = 0x%02X" DLM, systemVersion.hw);
        Serial.printf(" Type               = 0x%02X" DLM, systemVersion.type);
        Serial.printf(" Bootloader version = 0x%04X" DLM, systemVersion.fw);

        if (!lr11xx_is_fw_compatible_with_chip(LR11XX_FIRMWARE_UPDATE_TO, systemVersion.fw))
        {
            Serial.printf("ERROR: Bootloader is not compatible with chip." DLM);
            return -1;
        }

        // lr11xx_bootloader_pin_t pin;
        // lr11xx_bootloader_read_pin(wm1110Hw.radio.ral.context, pin);
        // lr11xx_bootloader_chip_eui_t chip_eui;
        // lr11xx_bootloader_read_chip_eui(wm1110Hw.radio.ral.context, chip_eui);
        // lr11xx_bootloader_join_eui_t join_eui;
        // lr11xx_bootloader_read_join_eui(wm1110Hw.radio.ral.context, join_eui);

        // Serial.printf(" PIN     = 0x%02X%02X%02X%02X" DLM, pin[0], pin[1], pin[2], pin[3]);
        // Serial.printf(" ChipEUI = 0x%02X%02X%02X%02X%02X%02X%02X%02X" DLM, chip_eui[0], chip_eui[1], chip_eui[2], chip_eui[3], chip_eui[4], chip_eui[5], chip_eui[6], chip_eui[7]);
        // Serial.printf(" JoinEUI = 0x%02X%02X%02X%02X%02X%02X%02X%02X" DLM, join_eui[0], join_eui[1], join_eui[2], join_eui[3], join_eui[4], join_eui[5], join_eui[6], join_eui[7]);
    }

    return 0;
}

static int lr11xx_enter_bootloader_mode( void )
{
    wm1110Hw.enterBootloaderMode();

    return 0;
}

// static int usrcmd_update(int argc, char **argv)
static int update_lr11xx_firmware( void )
{
    lr11xx_system_version_t systemVersion;
    if (lr11xx_system_get_version(wm1110Hw.radio.ral.context, &systemVersion) != LR11XX_STATUS_OK)
    {
        Serial.printf("ERROR: Failed to get system version." DLM);
        return -1;
    }

    if (!lr11xx_is_chip_in_production_mode(systemVersion.type))
    {
        Serial.printf("ERROR: Chip is NOT in bootloader mode." DLM);
        return -1;
    }
    else
    {
        if (!lr11xx_is_fw_compatible_with_chip(LR11XX_FIRMWARE_UPDATE_TO, systemVersion.fw))
        {
            Serial.printf("ERROR: Bootloader is not compatible with chip." DLM);
            return -1;
        }
    }

    Serial.printf("Start flash erase..." DLM);
    lr11xx_bootloader_erase_flash(wm1110Hw.radio.ral.context);
    Serial.printf("Flash erase done!" DLM);

    Serial.printf("Start flashing firmware..." DLM);
    lr11xx_bootloader_write_flash_encrypted_full(wm1110Hw.radio.ral.context, 0, lr11xx_firmware_image, LR11XX_FIRMWARE_IMAGE_SIZE);
    Serial.printf("Flashing done!" DLM);

    Serial.printf("Rebooting..." DLM);
    lr11xx_bootloader_reboot(wm1110Hw.radio.ral.context, false);
    Serial.printf("Reboot done!" DLM);

    switch( LR11XX_FIRMWARE_UPDATE_TO )
    {
        case LR1110_FIRMWARE_UPDATE_TO_TRX:
        case LR1120_FIRMWARE_UPDATE_TO_TRX:
        case LR1121_FIRMWARE_UPDATE_TO_TRX:
        {
            lr11xx_system_version_t version_trx = { 0x00 };

            lr11xx_system_get_version( wm1110Hw.radio.ral.context, &version_trx );
            Serial.printf( "Chip in transceiver mode:\n" );
            Serial.printf( " - Chip type             = 0x%02X\n", version_trx.type );
            Serial.printf( " - Chip hardware version = 0x%02X\n", version_trx.hw );
            Serial.printf( " - Chip firmware version = 0x%04X\n", version_trx.fw );

            break;
        }
        default:
            break;
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// setup and loop

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println();
    Serial.println();

    wm1110Hw.begin();
    lr11xx_system_reset(wm1110Hw.radio.ral.context);


}

void loop()
{
    Serial.printf("Chip ready to enter bootloader mode.\r\n");
    int ret = lr11xx_enter_bootloader_mode();

    ret = lr111xx_info_get();
    ret = update_lr11xx_firmware();
    while(1)
    {
        if(ret == LR11XX_STATUS_OK)
        {
            Serial.printf("Lr1110 update successed.\r\n");    
            delay(1000);        
        }
    }

    Serial.printf("Lr1110 update failed,  retry......\r\n");
    delay(EXECUTION_PERIOD);
}

////////////////////////////////////////////////////////////////////////////////
