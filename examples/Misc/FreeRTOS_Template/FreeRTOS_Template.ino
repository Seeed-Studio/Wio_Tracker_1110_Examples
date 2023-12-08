#include <Adafruit_TinyUSB.h> // for Serial


/*This is an example of freertos, which creates two tasks and outputs the running state of their respective tasks when the tasks are scheduled
*
*/

// task1
void task1(void *parameter)
{
    while (true)
    {
        Serial.println("Task1 running");
        vTaskDelay(1000);
    }
}
 
// task2
void task2(void *parameter)
{
    while (true)
    {
        Serial.println("Task2 running");
        vTaskDelay(2000);
    }
}
 
void setup()
{
    Serial.begin(115200);
    while (!Serial) delay(100);

    // create task
    xTaskCreate(task1, "Task 1", 256 * 4, NULL, 2, NULL);
    xTaskCreate(task2, "Task 2", 256 * 4, NULL, 2, NULL);
}

void loop()
{
    // On the Arduino platform, it is necessary to keep the loop running.
    delay(100);
}
