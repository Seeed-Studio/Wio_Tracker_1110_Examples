#include <Adafruit_TinyUSB.h> // for Serial
#include <Si115X.h>


Si115X si1151;

void setup()
{
    delay(100); // Wait for power on grove 

    // Initializes the debug output  
    Serial.begin(115200);
    while (!Serial) delay(100);     // Wait for ready

    Wire.begin();

    if (!si1151.Begin())
    {
        Serial.println("Si1151 is not ready!");
    }
    else
    {
        Serial.println("Si1151 is ready!");
    }
}

void loop()
{
    // Get datas
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
