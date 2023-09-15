#include "LIS3DHTR.h"
#include <Wire.h>
#include <Adafruit_TinyUSB.h> // for Serial

LIS3DHTR<TwoWire> LIS; //IIC
#define WIRE Wire


uint8_t flag = 0;


void int1_callback(void)
{

    Serial.println("LIS3DHTR  interrupt");
    LIS.getIntStatus(&flag);
    //3 axis
    Serial.print("x:"); Serial.print(LIS.getAccelerationX()); Serial.print("  ");
    Serial.print("y:"); Serial.print(LIS.getAccelerationY()); Serial.print("  ");
    Serial.print("z:"); Serial.println(LIS.getAccelerationZ());


}


void setup() 
{

    uint16_t error;
    char errorMessage[256];
    digitalWrite(PIN_POWER_SUPPLY_GROVE, HIGH);   //grove power on
    pinMode(PIN_POWER_SUPPLY_GROVE, OUTPUT);  

    delay(100);

  	pinMode(PIN_LIS3DHTR_INT2, INPUT);
	attachInterrupt(PIN_LIS3DHTR_INT2, int1_callback, ISR_DEFERRED | FALLING); //RISING

    Serial.begin(115200);
    while (!Serial) {
        delay(100);
    }

    // LIS.begin(WIRE); //IIC init dafault :0x18
    LIS.begin(WIRE, 0x19); //IIC init

    LIS.setInterrupt();


}
void loop() {

    if (!LIS) {
        Serial.println("LIS3DHTR didn't connect.");
        while (1);
        return;
    }
    // LIS.getIntStatus(&flag);
	Serial.print("x:"); Serial.print(LIS.getAccelerationX()); Serial.print("  ");
    Serial.print("y:"); Serial.print(LIS.getAccelerationY()); Serial.print("  ");
    Serial.print("z:"); Serial.println(LIS.getAccelerationZ());

	delay(500);
    
}



























