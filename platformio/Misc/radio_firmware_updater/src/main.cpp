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

#include <ntshell.h>
#include <util/ntopt.h>

////////////////////////////////////////////////////////////////////////////////
// Constants

static const char VERSION[] = "1.0";

#define DLM "\r\n"

static constexpr uint32_t EXECUTION_PERIOD = 50; // [msec.]

////////////////////////////////////////////////////////////////////////////////
// Variables

static Wm1110Hardware &wm1110Hw = Wm1110Hardware::getInstance();

static ntshell_t nts;

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
// NT-Shell handlers

static int usrcmd_ntopt_callback(int argc, char **argv, void *extobj);

static int serial_read(char *buf, int cnt, void *extobj)
{
    for (int i = 0; i < cnt; ++i)
    {
        const auto c = Serial.read();
        if (c < 0)
        {
            return i;
        }
        buf[i] = c;
    }

    return cnt;
}

static int serial_write(const char *buf, int cnt, void *extobj)
{
    return Serial.write(buf, cnt);
}

static int user_callback(const char *text, void *extobj)
{
    return ntopt_parse(text, usrcmd_ntopt_callback, extobj);
}

////////////////////////////////////////////////////////////////////////////////
// Commands implementation

typedef int (*USRCMDFUNC)(int argc, char **argv);

typedef struct
{
    const char *cmd;
    const char *desc;
    USRCMDFUNC func;
} cmd_table_t;

static int usrcmd_help(int argc, char **argv);
static int usrcmd_info(int argc, char **argv);
static int usrcmd_enter_bootloader_mode(int argc, char **argv);
static int usrcmd_update(int argc, char **argv);

static const cmd_table_t cmdlist[] = {
    {"help", "Show command list.", usrcmd_help},
    {"info", "Display radio chip information.", usrcmd_info},
    {"enter_bootloader_mode", "Transition to bootloader mode.", usrcmd_enter_bootloader_mode},
    {"update", "Update radio chip firmware.", usrcmd_update},
};

static int usrcmd_ntopt_callback(int argc, char **argv, void *extobj)
{
    if (argc < 1)
    {
        return 0;
    }

    for (int i = 0; i < static_cast<int>(std::extent<decltype(cmdlist)>::value); ++i)
    {
        if (strcmp(argv[0], cmdlist[i].cmd) == 0)
        {
            return cmdlist[i].func(argc, argv);
        }
    }

    Serial.println("Unknown command found.");
    return 0;
}

static int usrcmd_help(int argc, char **argv)
{
    Serial.print("radio_firmware_updater ");
    Serial.println(VERSION);
    Serial.printf("Containing firmware version = 0x%04X" DLM, LR11XX_FIRMWARE_VERSION);
    Serial.println();

    for (int i = 0; i < static_cast<int>(std::extent<decltype(cmdlist)>::value); ++i)
    {
        Serial.print(cmdlist[i].cmd);
        Serial.print("\t:");
        Serial.println(cmdlist[i].desc);
    }

    return 0;
}

static int usrcmd_info(int argc, char **argv)
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

static int usrcmd_enter_bootloader_mode(int argc, char **argv)
{
    wm1110Hw.enterBootloaderMode();

    return 0;
}

static int usrcmd_update(int argc, char **argv)
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

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// setup and loop

extern "C" void setup(void)
{
    Serial.begin(115200);
    delay(1000);
    Serial.println();
    Serial.println();

    wm1110Hw.begin();
    lr11xx_system_reset(wm1110Hw.radio.ral.context);

    ntshell_init(&nts, serial_read, serial_write, user_callback, nullptr);
    ntshell_set_prompt(&nts, "> ");
}

extern "C" void loop(void)
{
    ntshell_execute_nb(&nts);

    delay(EXECUTION_PERIOD);
}

////////////////////////////////////////////////////////////////////////////////
