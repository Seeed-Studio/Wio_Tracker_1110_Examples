# Wio Tracker 1110 Examples

Code examples for Wio Tracker 1110.

## Folder Structure

`/examples`

Contains examples for the Arduino IDE platform.

|Folder|Summary|Grove|LoRaWAN|RTOS|Note|
|:--|:--|:--|:--|:--|:--|
|[Grove/grove_analog](/examples/Grove/grove_analog)|An Arduino project reads analog voltage from the Grove - Analog (P1) sensor and outputs the voltage values to the Serial monitor in millivolts (mV)|Analog|||Analog|
|[Grove/Module/grove_barometer_dps310](/examples/Grove/Module/grove_barometer_dps310)|An Arduino project using a DPS310 pressure sensor to detect changes in air pressure, potentially indicating a fall and ouput to the Serial monitor|I2C|||DPS310|
|[Grove/Module/grove_accelerometer_lis3dhtr](/examples/Grove/Module/grove_accelerometer_lis3dhtr)|An arduino project that uses the iic interface to communicate with the LISDHTR accelerometer sensor, which outputs triaxial data and triggers interrupt states via Serial monitor|I2C|||LIS3DHTR|
|[Grove/Module/grove_gas_sgp41](/examples/Grove/Module/grove_gas_sgp41)|An arduino project that uses SHT41 and SGP41 sensors to measure air voc data and obtain an voc index by algorithm|I2C|||SHT41 SGP41|
|[Grove/Module/grove_temphumi_sht4x](/examples/Grove/Module/grove_temphumi_sht4x)|An arduino project that uses SHT41 sensors to measures temperature and humidity and output the values to the serial monitor|I2C|||SHT41|
|[Grove/Module/grove_light_si1151](/examples/Grove/Module/grove_light_si1151)|An arduino project that uses SI1151 sensors to measures Ultraviolet index value and output the values to the serial monitor|I2C|||SI1151|
|[Grove/Module/grove_sound](/examples/Grove/Module/grove_sound)|An arduino project that uses SOUND sensors to measures sound voltage value and output the values to the serial monitor|Analog|||SOUND|
|[Grove/Module/grove_distance_ultrasonic](/examples/Grove/Module/grove_distance_ultrasonic)|An arduino project that uses ULTRASONIC DISTANCE sensors to measures distance(unit:cm) and output the values to the serial monitor|Digital0|||ULTRASONIC DISTANCE|
|[LoRaWAN/SenseCAP/sensecap_ble_freertos](/examples/LoRaWAN/SenseCAP/sensecap_ble_freertos)|An Arduino project that collects various sensor data and scan for ble then uses freertos to run LoRaWAN upload datas to SenseCAP Cloud|I2C Analog Digital0|LoRaWAN|FreeRTOS|SenseCAP Cloud|
|[LoRaWAN/SenseCAP/sensecap_ble_uplink](/examples/LoRaWAN/SenseCAP/sensecap_ble_uplink)|An Arduino project that collects various sensor data and scan for ibeacon then using LoRaWAN upload to SenseCAP Cloud||LoRaWAN||SenseCAP Cloud|
|[LoRaWAN/SenseCAP/sensecap_gnss_freertos](/examples/LoRaWAN/SenseCAP/sensecap_gnss_freertos)|An Arduino project that collects various sensor data and scan for gnss then uses freertos to run LoRaWAN upload datas to SenseCAP Cloud|I2C Analog Digital0|LoRaWAN|FreeRTOS|SenseCAP Cloud|
|[LoRaWAN/SenseCAP/sensecap_gnss_uplink](/examples/LoRaWAN/SenseCAP/sensecap_gnss_uplink)|An Arduino project that collects various sensor data and scan for gnss then using LoRaWAN upload to SenseCAP Cloud||LoRaWAN||SenseCAP Cloud|
|[LoRaWAN/SenseCAP/sensecap_wifi_freertos](/examples/LoRaWAN/SenseCAP/sensecap_wifi_freertos)|An Arduino project that collects various sensor data and scan for wifi then uses freertos to run LoRaWAN upload datas to SenseCAP Cloud|I2C Analog Digital0|LoRaWAN|FreeRTOS|SenseCAP Cloud|
|[LoRaWAN/SenseCAP/sensecap_wifi_uplink](/examples/LoRaWAN/SenseCAP/sensecap_wifi_uplink)|An Arduino project that collects various sensor data and scan for wifi then using LoRaWAN upload to SenseCAP Cloud||LoRaWAN||SenseCAP Cloud|
|[LoRaWAN/SenseCAP/default_firmware](/examples/LoRaWAN/SenseCAP/default_firmware)|An Arduino project that collects various sensor data and scan for gnss then using LoRaWAN upload to SenseCAP Cloud||LoRaWAN||SenseCAP Cloud|
|[LoRaWAN/TheThingsNetwork/ttn_geolocation_gnss](/examples/LoRaWAN/TheThingsNetwork/ttn_geolocation_gnss)|An Arduino project that scan for gnss then using LoRaWAN upload to TTN Cloud||LoRaWAN||The Things Network|
|[LoRaWAN/TheThingsNetwork/ttn_uplink](/examples/LoRaWAN/TheThingsNetwork/ttn_uplink)|An Arduino project that using LoRaWAN upload count to TTN Cloud||LoRaWAN||The Things Network|
|[LoRaWAN/TheThingsNetwork/ttn_uplink_freertos](/examples/LoRaWAN/TheThingsNetwork/ttn_uplink_freertos)|An Arduino project that uses freertos to run LoRaWAN to upload data||LoRaWAN|FreeRTOS|The Things Network|
|[MISC/ble_scan_advanced](/examples/MISC/ble_scan_advanced)|An Arduino project that sets up a Bluetooth Low Energy (BLE) central device to scan for iBeacon advertisements and output the results to the serial monitor||||iBeacon|
|[MISC/freertos_template](/examples/MISC/freertos_template)|An Arduino project that uses freertos to create tasks|||FreeRTOS||
|[MISC/grove_i2c_scan](/examples/MISC/grove_i2c_scan)|An Arduino project scan I2C and output the I2C address that responded to the Serial monitor|I2C||||
|[MISC/p25q32h_flash_speedtest](/examples/MISC/p25q32h_flash_speedtest)|An Arduino project that tests the speed and functionality of a specific SPI Flash memory chip (P25Q32H) using an Adafruit library and output to the Serial monitor||||P25Q32H|
|[MISC/full_almanac_update](/examples/MISC/full_almanac_update)|An Arduino project that update almanac to Lr1110 and output almanac date to the Serial monitor|||||
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
