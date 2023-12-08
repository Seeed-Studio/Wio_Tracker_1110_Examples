#include <Adafruit_TinyUSB.h> // for Serial
#include <Wire.h>
#include <Ultrasonic.h>

constexpr int ULTRASONIC_PIN = D0;

Ultrasonic ultrasonic(ULTRASONIC_PIN);

void setup()
{
    delay(100); // Wait for power on grove

    // Initializes the debug output    
    Serial.begin(115200);
    while (!Serial) delay(100);     // Wait for ready
}

void loop()
{
    long RangeInInches;
    long RangeInCentimeters;

    Serial.println("The distance to obstacles in front is: ");
    RangeInInches = ultrasonic.MeasureInInches();
    Serial.print(RangeInInches);  //0~157 inches
    Serial.println(" inch");
    delay(250);

    RangeInCentimeters = ultrasonic.MeasureInCentimeters(); // two measurements should keep an interval
    Serial.print(RangeInCentimeters); //0~400cm
    Serial.println(" cm");
    delay(2500);
}
