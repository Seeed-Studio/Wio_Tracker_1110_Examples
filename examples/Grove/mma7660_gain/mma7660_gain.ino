#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_TinyUSB.h> // for Serial

#include <grove_sensor.hpp>

int interruptPin = D0;

MMA7660 accelemeter;

void digital_callback(void)
{
	MMA7660_DATA data;
	Serial1.print("Pin value: ");
	Serial1.println(digitalRead(interruptPin));

	Serial1.print("TILT: ");
	accelemeter.getAllData(&data);
	Serial1.print(data.TILT);

}


void setup()
{
    //power on 
    digitalWrite(PIN_POWER_SUPPLY_GROVE, HIGH);   //grove power on
    pinMode(PIN_POWER_SUPPLY_GROVE, OUTPUT);  
	Serial1.begin(115200);

	delay(1000);

	accelemeter.init(MMA7660_SHINTX|MMA7660_SHINTY|MMA7660_SHINTZ);  
	
  	pinMode(interruptPin, INPUT_PULLUP);
	attachInterrupt(interruptPin, digital_callback, ISR_DEFERRED | FALLING);

}
void loop()
{
	int8_t x;
	int8_t y;
	int8_t z;
	float ax,ay,az;
	accelemeter.getXYZ(&x,&y,&z);
	
	Serial1.print("x = ");
    Serial1.println(x); 
    Serial1.print("y = ");
    Serial1.println(y);   
    Serial1.print("z = ");
    Serial1.println(z);
	
	accelemeter.getAcceleration(&ax,&ay,&az);
    Serial1.println("accleration of X/Y/Z: ");
	Serial1.print(ax);
	Serial1.println(" g");
	Serial1.print(ay);
	Serial1.println(" g");
	Serial1.print(az);
	Serial1.println(" g");
	Serial1.println("*************");
	delay(500);
}


