#include <Arduino.h>
#include <Adafruit_TinyUSB.h> // for Serial


// task1
void task1(void *parameter) {
  while (true) {
	Serial.println("Task1 running");
    vTaskDelay(1000);  // 
  }
}
 
// task2
void task2(void *parameter) {
  while (true) {
	Serial.println("Task2 running");

    vTaskDelay(2000);  // 
  }
}
 
void setup() {

	// hardware init
	Serial.begin(115200);
	while (!Serial) {
		delay(100);
	}

	// create task
	xTaskCreate(task1, "Task 1", 256*4, NULL, 2, NULL);
	xTaskCreate(task2, "Task 2", 256*4, NULL, 2, NULL);

	vTaskStartScheduler();
}
 
void loop() {

}
