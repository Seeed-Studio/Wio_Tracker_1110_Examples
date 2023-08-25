# Wio Tracker 1110 Examples

Code examples for Wio Tracker 1110.

## Folder Structure

`/examples`

Contains examples for the Arduino IDE platform.

|Folder|Summary|LoRaWAN|Note|
|:--|:--|:--|:--|
|[Grove/adc_gain](/examples/Grove/adc_gain)|Output Grove - Analog (P1) voltage to Grove - UART.||Grove - Analog|
|[Grove/iic_scan](/examples/Grove/iic_scan)|Scan I2C and output the I2C address that responded to Grove - UART.||Grove - I2C|
|[Grove/mma7660_gain](/examples/Grove/mma7660_gain)|Output the accelerometer(MMA7660) value to Grove - UART.||MMA7660|
|[Grove/p25q32h_flash_speedtest](/examples/Grove/p25q32h_flash_speedtest)||||
|[Grove/temperature_gain](/examples/Grove/temperature_gain)|Output the temperature and humidity sensor(SHT4x) value to Grove - UART.||SHT4x|
|[BLE/ble_scan_advanced](/examples/BLE/ble_scan_advanced)||||
|[LoRaWAN/SenseCAP/sensecap_ble_uplink](/examples/LoRaWAN/SenseCAP/sensecap_ble_uplink)||SenseCAP Cloud||
|[LoRaWAN/SenseCAP/sensecap_gnss_uplink](/examples/LoRaWAN/SenseCAP/sensecap_gnss_uplink)||SenseCAP Cloud||
|[LoRaWAN/SenseCAP/sensecap_wifi_uplink](/examples/LoRaWAN/SenseCAP/sensecap_wifi_uplink)||SenseCAP Cloud||
|[LoRaWAN/TheThingsNetwork/ttn_geolocation_gnss](/examples/LoRaWAN/TheThingsNetwork/ttn_geolocation_gnss)||The Things Network||
|[LoRaWAN/TheThingsNetwork/ttn_uplink](/examples/LoRaWAN/TheThingsNetwork/ttn_uplink)||The Things Network||

`/platformio`

Contains examples for the PlatformIO platform.

|Folder|Summary|LoRaWAN|Note|
|:--|:--|:--|:--|
|[Grove/io_test](/platformio/Grove/io_test)||||
|[LoRaWAN/TheThingsNetwork/ttn_geolocation_gnss](/platformio/LoRaWAN/TheThingsNetwork/ttn_geolocation_gnss)||The Things Network||
|[LoRaWAN/TheThingsNetwork/ttn_geolocation_wifi](/platformio/LoRaWAN/TheThingsNetwork/ttn_geolocation_wifi)||The Things Network||
|[LoRaWAN/TheThingsNetwork/ttn_uplink](/platformio/LoRaWAN/TheThingsNetwork/ttn_uplink)||The Things Network||
|[LoRaWAN/TheThingsNetwork/ttn_uplink_with_beacon](/platformio/LoRaWAN/TheThingsNetwork/ttn_uplink_with_beacon)||The Things Network||


`/src`

Contains a dummy header file for display in the Examples menu of the Arduino IDE.

## Technical Notes

### Firmware Overview

<img width="400" src="media/1.png">

### Internal Flash Memory Map

<img width="400" src="media/2.png">

## License

[MIT](LICENSE.txt)
