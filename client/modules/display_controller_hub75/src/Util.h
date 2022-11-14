#include <Arduino.h>
#include <Esp.h>

void outputMemoryData(){
    log_v("HEAP: %d / %d; minfree: %d; maxalloc: %d", 
        ESP.getFreeHeap(), 
        ESP.getHeapSize(), 
        ESP.getMinFreeHeap(), 
        ESP.getMaxAllocHeap());
        
    // Serial.print("*** HEAP: free: ");
    // Serial.print(ESP.getFreeHeap());
    // Serial.print(" / ");
    // Serial.print(ESP.getHeapSize());
    // Serial.print("; minfree: ");
    // Serial.print(ESP.getMinFreeHeap());
    // Serial.print("; maxalloc: ");
    // Serial.println(ESP.getMaxAllocHeap());
}


