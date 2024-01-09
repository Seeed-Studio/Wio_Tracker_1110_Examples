# radio_firmware_updater

radio_firmware_updater is a tool that updates the firmware of LR1110 built into the WM1110 module included in Wio Tracker 1110.
This tool can display LR1110 chip information, enter bootloader mode, and update firmware.
You can operate from a USB serial terminal.

## Firmware update procedure

1. Confirm that it is the firmware version you want to update using the `help` command.

    ```
    > help
    ...
    Containing firmware version = 0x0307
    ...
    ```

2. Confirm that it is in bootloader mode using the `info` command.
When in application mode, execute `enter_bootloader_mode` command and check again with the `info` command.

    ```
    > info
    Chip is in bootloader mode.
    ...
    ```

3. Run the `update` command to update the firmware.

    ```
    > update
    ...
    ```

4. Check whether the firmware version has been updated using the `info` command.

    ```
    > info
    Chip is in firmware mode.
    ...
     Firmware version = 0x0307
    ```

## Command reference

* `help`

    Show command list.
    The tool version and containing firmware version are also displayed.

    ```
    > help
    radio_firmware_updater 1.0
    Containing firmware version = 0x0307

    help    :Show command list.
    info    :Display radio chip information.
    enter_bootloader_mode   :Transition to bootloader mode.
    update  :Update radio chip firmware.
    ```

* `info`

    Display radio chip information.

    **in firmware mode:**
    ```
    > info
    Chip is in firmware mode.
    Chip:
     Hardware version = 0x22
     Type             = 0x01
     Firmware version = 0x0307
    ```

    **in bootloader mode:**
    ```
    > info
    Chip is in bootloader mode.
    Chip:
     Hardware version   = 0x22
     Type               = 0xDF
     Bootloader version = 0x6500
    ```

* `enter_bootloader_mode`

    Transition to bootloader mode.

* `update`

    Update radio chip firmware.

    ```
    > update
    Start flash erase...
    Flash erase done!
    Start flashing firmware...
    Flashing done!
    Rebooting...
    Reboot done!
    ```

## Appendix

### .hex to .uf2

```
cd .pio
git clone https://github.com/microsoft/uf2
cd uf2\utils
python uf2conv.py ..\..\build\release\firmware.hex --family 0xADA52840 --convert --output ..\..\build\release\firmware.uf2
```
