#include <Arduino.h>
#include <Adafruit_TinyUSB.h> // for Serial

#include <grove_sensor.hpp>

int adcin    = D6;
int adcvalue = 0;
float mv_per_lsb = 3600.0F/1024.0F; // 10-bit ADC with 3.6V input range

void setup() {
  digitalWrite(PIN_POWER_SUPPLY_GROVE, HIGH);   //grove power on
  pinMode(PIN_POWER_SUPPLY_GROVE, OUTPUT);  

  Serial1.begin(115200);
  while ( !Serial1 ) delay(10);   // for nrf52840 with native usb
}

void loop() {
  // Get a fresh ADC value
  adcvalue = analogRead(adcin);

  // Display the results
  Serial1.print(adcvalue);
  Serial1.print(" [");
  Serial1.print((float)adcvalue * mv_per_lsb);
  Serial1.println(" mV]");

  delay(1000);
}
