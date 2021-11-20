#include <Arduino.h>
#include <Esp.h>

void outputMemoryData(){
    Serial.print("*** HEAP: free: ");
    Serial.print(ESP.getFreeHeap());
    Serial.print(" / ");
    Serial.print(ESP.getHeapSize());
    Serial.print("; minfree: ");
    Serial.print(ESP.getMinFreeHeap());
    Serial.print("; maxalloc: ");
    Serial.println(ESP.getMaxAllocHeap());
}


