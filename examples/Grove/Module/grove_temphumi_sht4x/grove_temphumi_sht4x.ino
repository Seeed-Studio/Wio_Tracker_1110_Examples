#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_TinyUSB.h> 

#include "SensirionI2CSht4x.h"

SensirionI2CSht4x sht4x;

void setup() {
    //power on 
    digitalWrite(PIN_POWER_SUPPLY_GROVE, HIGH);   //grove power on
    pinMode(PIN_POWER_SUPPLY_GROVE, OUTPUT);  

    Serial.begin(115200);
    while (!Serial) {
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
        Serial.print("Error trying to execute serialNumber(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    } else {
        Serial.print("Serial Number: ");
        Serial.println(serialNumber);
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
        Serial.print("Error trying to execute measureHighPrecision(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    } else {
        Serial.print("Temperature:");
        Serial.print(temperature);
        Serial.print("\t");
        Serial.print("Humidity:");
        Serial.println(humidity);
    }
}
