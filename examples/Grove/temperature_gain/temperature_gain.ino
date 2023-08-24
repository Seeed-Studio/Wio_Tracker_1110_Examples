#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_TinyUSB.h> // for Serial

#include <grove_sensor.hpp>


SensirionI2CSht4x sht4x;

void setup() {
    //power on 
    digitalWrite(PIN_POWER_SUPPLY_GROVE, HIGH);   //grove power on
    pinMode(PIN_POWER_SUPPLY_GROVE, OUTPUT);  

    Serial1.begin(115200);
    while (!Serial1) {
        delay(100);
    }

    Wire.begin();

    uint16_t error;
    char errorMessage[256];
    delay(1000);

    sht4x.begin(Wire);

    uint32_t serialNumber;
    error = sht4x.serialNumber(serialNumber);
    if (error) {
        Serial1.print("Error trying to execute serialNumber(): ");
        errorToString(error, errorMessage, 256);
        Serial1.println(errorMessage);
    } else {
        Serial1.print("Serial Number: ");
        Serial1.println(serialNumber);
    }
}

void loop() {
    uint16_t error;
    char errorMessage[256];

    delay(1000);

    float temperature;
    float humidity;
    error = sht4x.measureHighPrecision(temperature, humidity);
    if (error) {
        Serial1.print("Error trying to execute measureHighPrecision(): ");
        errorToString(error, errorMessage, 256);
        Serial1.println(errorMessage);
    } else {
        Serial1.print("Temperature:");
        Serial1.print(temperature);
        Serial1.print("\t");
        Serial1.print("Humidity:");
        Serial1.println(humidity);
    }
}
