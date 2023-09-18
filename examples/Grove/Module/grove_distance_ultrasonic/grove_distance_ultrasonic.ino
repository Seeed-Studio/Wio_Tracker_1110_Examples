#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_TinyUSB.h> 

#include "Ultrasonic.h"

int ultrasonic_pin    = D0;

Ultrasonic ultrasonic(ultrasonic_pin);
void setup() {
	digitalWrite(PIN_POWER_SUPPLY_GROVE, HIGH);   //grove power on
	pinMode(PIN_POWER_SUPPLY_GROVE, OUTPUT);  
	delay(100);

	Serial.begin(115200);
	while (!Serial) {
		delay(100);
	} 
}
void loop() {
	long RangeInInches;
	long RangeInCentimeters;

	Serial.println("The distance to obstacles in front is: ");
	RangeInInches = ultrasonic.MeasureInInches();
	Serial.print(RangeInInches);//0~157 inches
	Serial.println(" inch");
	delay(250);

	RangeInCentimeters = ultrasonic.MeasureInCentimeters(); // two measurements should keep an interval
	Serial.print(RangeInCentimeters);//0~400cm
	Serial.println(" cm");
	delay(2500);
}