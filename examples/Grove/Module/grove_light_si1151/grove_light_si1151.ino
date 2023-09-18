#include <Adafruit_TinyUSB.h> 

#include "Si115X.h"

Si115X si1151;

/**
 * Setup for configuration
 */
void setup()
{
    uint8_t conf[4];
    digitalWrite(PIN_POWER_SUPPLY_GROVE, HIGH);   //grove power on
    pinMode(PIN_POWER_SUPPLY_GROVE, OUTPUT);  

    delay(100);

    Wire.begin();

    Serial.begin(115200);
    while ( !Serial ) delay(10);   // for nrf52840 with native usb

    if (!si1151.Begin())
        Serial.println("Si1151 is not ready!");
    else
        Serial.println("Si1151 is ready!");

}

/**
 * Loops and reads data from registers
 */
void loop()
{
    Serial.print("IR: ");
    Serial.println(si1151.ReadHalfWord());
    Serial.print("VISIBLE: ");
    delay(1);
    Serial.println(si1151.ReadHalfWord_VISIBLE());
    Serial.print("UV: ");
    delay(1);
    Serial.println(si1151.ReadHalfWord_UV());
    delay(100);
}