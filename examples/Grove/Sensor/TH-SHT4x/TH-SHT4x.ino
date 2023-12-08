#include <Adafruit_TinyUSB.h> // for Serial
#include <Wire.h>
#include <SensirionI2CSht4x.h>

SensirionI2CSht4x sht4x;

void setup()
{
    delay(100); // Wait for power on grove 

    // Initializes the debug output
    Serial.begin(115200);
    while (!Serial) delay(100);     // Wait for ready

    Wire.begin();

    uint16_t error;
    char errorMessage[256];
    delay(1000);

    // Initializes the SHT4x
    sht4x.begin(Wire);

    uint32_t serialNumber;
    error = sht4x.serialNumber(serialNumber); // Get SHT4x's serialNumber
    if (error)
    {
        Serial.print("Error trying to execute serialNumber(): ");
        errorToString(error, errorMessage, sizeof(errorMessage));
        Serial.println(errorMessage);
    }
    else
    {
        Serial.print("Serial Number: ");
        Serial.println(serialNumber);
    }
}

void loop()
{
    uint16_t error;
    char errorMessage[256];

    delay(1000);
    
    // Get temperature & humidity datas
    float temperature;
    float humidity;
    error = sht4x.measureHighPrecision(temperature, humidity); 
    if (error)
    {
        Serial.print("Error trying to execute measureHighPrecision(): ");
        errorToString(error, errorMessage, sizeof(errorMessage));
        Serial.println(errorMessage);
    }
    else
    {
        Serial.print("Temperature:");
        Serial.print(temperature);
        Serial.print("\t");
        Serial.print("Humidity:");
        Serial.println(humidity);
    }
}
