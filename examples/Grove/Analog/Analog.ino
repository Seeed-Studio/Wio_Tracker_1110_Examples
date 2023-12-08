#include <Adafruit_TinyUSB.h> // for Serial

constexpr int ADCIN = A0;
constexpr float MV_PER_LSB = 3600.0f / 1024.0f; // 10-bit ADC with 3.6V input range

void setup()
{
    delay(100); // Wait for power on grove   
    
    // Initializes the debug output
    Serial.begin(115200);
    while (!Serial) delay(100);     // Wait for ready
}

void loop()
{
    // Get a fresh ADC value
    long sum = 0;
    for (int i = 0; i < 32; i++) // Sample multiple times to average
    {
        sum += analogRead(ADCIN);
    }
    int adcvalue = sum / 32;

    // Display the results
    Serial.print(adcvalue);
    Serial.print(" [");
    Serial.print((float)adcvalue * MV_PER_LSB);
    Serial.println(" mV]");

    delay(1000);
}
