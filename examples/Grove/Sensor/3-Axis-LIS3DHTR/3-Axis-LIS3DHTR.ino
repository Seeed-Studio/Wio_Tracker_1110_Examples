#include <Adafruit_TinyUSB.h> // for Serial
#include <Wire.h>
#include <LIS3DHTR.h>

#define WIRE Wire


LIS3DHTR<TwoWire> LIS; //IIC

// The interrupt callback function
void int1_callback()
{
    Serial.println("LIS3DHTR  interrupt");

    uint8_t flag = 0;
    LIS.getIntStatus(&flag); // Get the interrupt status and clear it

    //3 axis
    Serial.print("x:"); Serial.print(LIS.getAccelerationX()); Serial.print("  ");
    Serial.print("y:"); Serial.print(LIS.getAccelerationY()); Serial.print("  ");
    Serial.print("z:"); Serial.println(LIS.getAccelerationZ());
}

void setup() 
{
    delay(100); // Wait for power on grove 

    // Initializes the debug output  
    Serial.begin(115200);
    while (!Serial) delay(100);     // Wait for ready

    // Initializes the interrupt pin
    pinMode(PIN_LIS3DHTR_INT2, INPUT);
    attachInterrupt(PIN_LIS3DHTR_INT2, int1_callback, ISR_DEFERRED | FALLING); //RISING

    // Initializes LIS3DHTR
    LIS.begin(WIRE, 0x19); 

    // Configure LIS3DHTR to trigger an interrupt
    LIS.setInterrupt();
}

void loop()
{
    if (!LIS) // LIS3DHTR is not ready
    {
        Serial.println("LIS3DHTR didn't connect.");
        while (1);
    }

    Serial.print("x:"); Serial.print(LIS.getAccelerationX()); Serial.print("  ");
    Serial.print("y:"); Serial.print(LIS.getAccelerationY()); Serial.print("  ");
    Serial.print("z:"); Serial.println(LIS.getAccelerationZ());

    delay(500);
}
