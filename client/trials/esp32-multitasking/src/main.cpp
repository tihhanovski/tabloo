#include <Arduino.h>

volatile int i = 0;
char buffer[1024];
const char* msg = "test %d \0";

TaskHandle_t myTaskHandle;
TaskHandle_t myTaskHandle2;

void myTask(void * pvParameters) {

    log_v("Task started on %d", xPortGetCoreID());

    unsigned long l = 0;

    while(true) {
        unsigned long t = millis();
        if(t > l) {
            int s = i + 1;
            log_v("started overwriting i with %d", s);
            l = t + 450;
            while(millis() < l) {
                i = s;
                sprintf(buffer, msg, i);
                vTaskDelay(1);
            }
            l = t + 900;
            log_v("%d: core %d >>\t'%s'", i, xPortGetCoreID(), buffer);
            log_v("finished changing");
            vTaskDelay(1);
        }
    }
}

void setup() {
    Serial.begin(115200);

    xTaskCreatePinnedToCore(
                    myTask,             /* Task function. */
                    "myTask",           /* name of task. */
                    10000,              /* Stack size of task */
                    NULL,               /* parameter of the task */
                    0,                  /* priority of the task */
                    &myTaskHandle       /* Task handle to keep track of created task */
                    , 0);
    log_v("Started task");


    xTaskCreatePinnedToCore(
                    myTask,             /* Task function. */
                    "myTask2",           /* name of task. */
                    10000,              /* Stack size of task */
                    NULL,               /* parameter of the task */
                    0,                  /* priority of the task */
                    &myTaskHandle2       /* Task handle to keep track of created task */
                    , 1);
    log_v("Started task");
}

unsigned long nextDec = 0;

void loop() {
    unsigned long t = millis();
    if(t > nextDec) {
        nextDec = t + 2000;
        i--;
        sprintf(buffer, msg, i);
        log_v("%d: core %d >>\t'%s'", i, xPortGetCoreID(), buffer);
    }
}