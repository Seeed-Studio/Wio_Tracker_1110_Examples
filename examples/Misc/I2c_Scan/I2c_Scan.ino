#include <Adafruit_TinyUSB.h> // for Serial
#include <Wire.h>

TwoWire *wi = &Wire;

/*
This is an example of scanning  IIC slave devices which is connecting to the I2C grove
*/

void setup()
{
    delay(100);
    Serial.begin(115200);
    while (!Serial) delay(100);

    wi->begin();        // join i2c bus (address optional for main)
}

void loop()
{
    Serial.println("Scanning address from 0 to 127:");
    for (int addr = 1; addr < 128; addr++)
    {
        wi->beginTransmission(addr);
        if (wi->endTransmission() == 0)
        {
            switch (addr)
            {
                case 0x44:
                    Serial.print("Found sht40 slave addr: 0x");
                    break;
                case 0x19:
                    Serial.print("Found lis3dhtr slave addr: 0x");
                    break;
                case 0x53:
                case 0x52:
                    Serial.print("Found Sunlight sensor slave addr: 0x");
                    break;
                case 0x59:
                    Serial.print("Found sgp41 slave addr: 0x");
                    break;
                case 0x77:
                    Serial.print("Found dps310 slave addr: 0x");
                    break;
                default:
                    Serial.print("Found: 0x");
                    break;
            }
            Serial.print(addr, HEX);
            Serial.println();
        }
    }
    Serial.println();
    delay(5000);
}
