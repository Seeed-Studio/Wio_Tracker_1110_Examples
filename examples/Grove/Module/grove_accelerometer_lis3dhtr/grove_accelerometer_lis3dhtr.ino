#include <Adafruit_TinyUSB.h> // for Serial
#include <Wire.h>
#include <LIS3DHTR.h>

#define WIRE Wire

LIS3DHTR<TwoWire> LIS; //IIC

void int1_callback()
{
  Serial.println("LIS3DHTR  interrupt");
  
  uint8_t flag = 0;
  LIS.getIntStatus(&flag);

  //3 axis
  Serial.print("x:"); Serial.print(LIS.getAccelerationX()); Serial.print("  ");
  Serial.print("y:"); Serial.print(LIS.getAccelerationY()); Serial.print("  ");
  Serial.print("z:"); Serial.println(LIS.getAccelerationZ());
}

void setup() 
{
  delay(100);
  Serial.begin(115200);
  while (!Serial) delay(100);

  pinMode(PIN_LIS3DHTR_INT2, INPUT);
  attachInterrupt(PIN_LIS3DHTR_INT2, int1_callback, ISR_DEFERRED | FALLING); //RISING

  LIS.begin(WIRE, 0x19); //IIC init
  LIS.setInterrupt();
}

void loop()
{
  if (!LIS)
  {
    Serial.println("LIS3DHTR didn't connect.");
    while (1);
  }

  Serial.print("x:"); Serial.print(LIS.getAccelerationX()); Serial.print("  ");
  Serial.print("y:"); Serial.print(LIS.getAccelerationY()); Serial.print("  ");
  Serial.print("z:"); Serial.println(LIS.getAccelerationZ());

  delay(500);
}
