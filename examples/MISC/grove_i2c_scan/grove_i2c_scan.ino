#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_TinyUSB.h> // for Serial


TwoWire *wi = &Wire;

void setup()
{
    //power on 
    digitalWrite(PIN_POWER_SUPPLY_GROVE, HIGH);   //grove power on
    pinMode(PIN_POWER_SUPPLY_GROVE, OUTPUT);  

	
    Serial.begin(115200);
    while (!Serial) {
        delay(100);
    }
	wi->begin();        // join i2c bus (address optional for main)
}

void loop()
{
	Serial.println("Scanning address from 0 to 127:");
	for (int addr = 1; addr < 128; addr++)
	{
		wi->beginTransmission(addr);
		if ( 0 == wi->endTransmission() )
		{
			if(addr==0x44)
			{
				Serial.print("Found sht40 slave addr: 0x");
			}
			else if(addr==0x19)
			{
				Serial.print("Found lis3dhtr slave addr: 0x");
			}
			else if(addr==0x53 || addr==0x52)
			{
				Serial.print("Found Sunlight sensor slave addr: 0x");
			}
			else if(addr==0x59)
			{
				Serial.print("Found sgp41 slave addr: 0x");
			}
			else if(addr==0x77)
			{
				Serial.print("Found dps310 slave addr: 0x");
			}
			else
			{
				Serial.print("Found: 0x");
			}
			Serial.print(addr, HEX);
			Serial.println();
		}
	}
	Serial.println();
	delay(5000);
}



