#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_TinyUSB.h> // for Serial

#include <grove_sensor.hpp>

TwoWire *wi = &Wire;

void setup()
{
    //power on 
    digitalWrite(PIN_POWER_SUPPLY_GROVE, HIGH);   //grove power on
    pinMode(PIN_POWER_SUPPLY_GROVE, OUTPUT);  

	
	Serial1.begin(115200);  // start serial for output
	while ( !Serial1 ) delay(10);   // for nrf52840 with native usb
	wi->begin();        // join i2c bus (address optional for main)
}

void loop()
{
	Serial1.println("Scanning address from 0 to 127");
	for (int addr = 1; addr < 128; addr++)
	{
		wi->beginTransmission(addr);
		if ( 0 == wi->endTransmission() )
		{
			if(addr==0x44)
			{
				Serial1.print("Found sht40 slave addr: 0x");
				Serial1.print(addr, HEX);
				Serial1.println();
			}
			else if(addr==0x44)
			{
				Serial1.print("Found mma7660 slave addr: 0x");
				Serial1.print(addr, HEX);
				Serial1.println();
			}
			else
			{
				Serial1.print("Found: 0x");
				Serial1.print(addr, HEX);
				Serial1.println();
			}
		}
	}

	delay(5000);
}



