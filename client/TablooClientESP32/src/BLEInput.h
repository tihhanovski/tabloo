/**
 * Tabloo - opensource bus stop display
 * BLE communication to setup tabloo
 * see 
 * https://h2zero.github.io/esp-nimble-cpp/index.html
 * https://github.com/h2zero/esp-nimble-cpp
 * 
 * @author Ilja Tihhanovski <ilja.tihhanovski@gmail.com>
 * 
 * TODO: very basic and raw code. Refactor! 
*/

#include <Arduino.h>
#include <NimBLEDevice.h>

#define MAX_BLE_COMMAND_SIZE 100

void (*ble_onCommandReceived)(char* command) = nullptr;           // Command received

static NimBLEServer* pServer;

/**  None of these are required as they will be handled by the library with defaults. **
 **                       Remove as you see fit for your needs                        */  
class ServerCallbacks: public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) {
        Serial.println("Client connected");
        Serial.println("Multi-connect support: start advertising");
        NimBLEDevice::startAdvertising();
    };
    /** Alternative onConnect() method to extract details of the connection. 
     *  See: src/ble_gap.h for the details of the ble_gap_conn_desc struct.
     */  
    void onConnect(NimBLEServer* pServer, ble_gap_conn_desc* desc) {
        Serial.print("Client address: ");
        Serial.println(NimBLEAddress(desc->peer_ota_addr).toString().c_str());
        /** We can use the connection handle here to ask for different connection parameters.
         *  Args: connection handle, min connection interval, max connection interval
         *  latency, supervision timeout.
         *  Units; Min/Max Intervals: 1.25 millisecond increments.
         *  Latency: number of intervals allowed to skip.
         *  Timeout: 10 millisecond increments, try for 5x interval time for best results.  
         */
        pServer->updateConnParams(desc->conn_handle, 24, 48, 0, 60);
    };
    void onDisconnect(NimBLEServer* pServer) {
        Serial.println("Client disconnected - start advertising");
        NimBLEDevice::startAdvertising();
    };
    void onMTUChange(uint16_t MTU, ble_gap_conn_desc* desc) {
        Serial.printf("MTU updated: %u for connection ID: %u\n", MTU, desc->conn_handle);
    };
    
/********************* Security handled here **********************
****** Note: these are the same return values as defaults ********/
    uint32_t onPassKeyRequest(){
        Serial.println("Server Passkey Request");
        /** This should return a random 6 digit number for security 
         *  or make your own static passkey as done here.
         */
        return 123456; 
    };

    bool onConfirmPIN(uint32_t pass_key){
        Serial.print("The passkey YES/NO number: ");Serial.println(pass_key);
        /** Return false if passkeys don't match. */
        return (pass_key == 2021); 
    };

    void onAuthenticationComplete(ble_gap_conn_desc* desc){
        /** Check that encryption was successful, if not we disconnect the client */  
        if(!desc->sec_state.encrypted) {
            NimBLEDevice::getServer()->disconnect(desc->conn_handle);
            Serial.println("Encrypt connection failed - disconnecting client");
            return;
        }
        Serial.println("Starting BLE work!");
    };
};

/** Handler class for characteristic actions */
class CharacteristicCallbacks: public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic* pCharacteristic){
        Serial.print(pCharacteristic->getUUID().toString().c_str());
        Serial.print(": onRead(), value: ");
        Serial.println(pCharacteristic->getValue().c_str());
    };

    void onWrite(NimBLECharacteristic* pCharacteristic) {
        Serial.print(pCharacteristic->getUUID().toString().c_str());
        Serial.print(": onWrite(), value: ");
        Serial.println(pCharacteristic->getValue().c_str());

         

        if(ble_onCommandReceived != nullptr)
            ble_onCommandReceived((char*)(pCharacteristic->getValue().c_str()));


    };
    /** Called before notification or indication is sent, 
     *  the value can be changed here before sending if desired.
     */
    void onNotify(NimBLECharacteristic* pCharacteristic) {
        Serial.println("Sending notification to clients");
    };


    /** The status returned in status is defined in NimBLECharacteristic.h.
     *  The value returned in code is the NimBLE host return code.
     */
    void onStatus(NimBLECharacteristic* pCharacteristic, Status status, int code) {
        String str = ("Notification/Indication status code: ");
        str += status;
        str += ", return code: ";
        str += code;
        str += ", "; 
        str += NimBLEUtils::returnCodeToString(code);
        Serial.println(str);
    };

    void onSubscribe(NimBLECharacteristic* pCharacteristic, ble_gap_conn_desc* desc, uint16_t subValue) {
        String str = "Client ID: ";
        str += desc->conn_handle;
        str += " Address: ";
        str += std::string(NimBLEAddress(desc->peer_ota_addr)).c_str();
        if(subValue == 0) {
            str += " Unsubscribed to ";
        }else if(subValue == 1) {
            str += " Subscribed to notfications for ";
        } else if(subValue == 2) {
            str += " Subscribed to indications for ";
        } else if(subValue == 3) {
            str += " Subscribed to notifications and indications for ";
        }
        str += std::string(pCharacteristic->getUUID()).c_str();

        Serial.println(str);
    };
};
    
/** Handler class for descriptor actions */    
class DescriptorCallbacks : public NimBLEDescriptorCallbacks {
    void onWrite(NimBLEDescriptor* pDescriptor) {
        std::string dscVal((char*)pDescriptor->getValue(), pDescriptor->getLength());
        Serial.print("Descriptor witten value:");
        Serial.println(dscVal.c_str());
    };

    void onRead(NimBLEDescriptor* pDescriptor) {
        Serial.print(pDescriptor->getUUID().toString().c_str());
        Serial.println(" Descriptor read");
    };
};


/** Define callback instances globally to use for multiple Charateristics \ Descriptors */ 
static DescriptorCallbacks dscCallbacks;
static CharacteristicCallbacks chrCallbacks;


void startBleSetup() {
    Serial.println("Starting NimBLE Server");

    /** sets device name */
    NimBLEDevice::init("Tabloo_v1");
    Serial.println("initialized");
    Serial.flush();

    NimBLEDevice::setSecurityAuth(/*BLE_SM_PAIR_AUTHREQ_BOND | BLE_SM_PAIR_AUTHREQ_MITM |*/ BLE_SM_PAIR_AUTHREQ_SC);

    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());


    Serial.println("server created");
    Serial.flush();
  
    NimBLEService* pSetupService = pServer->createService("AAAA");
    NimBLECharacteristic* pSetupCharacteristic = pSetupService->createCharacteristic(
                                               "AAAB",
                                               NIMBLE_PROPERTY::READ |
                                               NIMBLE_PROPERTY::WRITE |
                                               NIMBLE_PROPERTY::NOTIFY
                                              );

    Serial.println("service created");
    Serial.flush();

    pSetupCharacteristic->setValue("--");
    pSetupCharacteristic->setCallbacks(&chrCallbacks);

    Serial.println("callback set");
    Serial.flush();

    pSetupService->start();
    Serial.println("service started");
    Serial.flush();

    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(pSetupService->getUUID());
    pAdvertising->setScanResponse(true);
    pAdvertising->start();

    Serial.println("Advertising Started");
}

//unsigned long ble_loop_timer = 0;

void ble_loop() {
  /** Do your thing here, this just spams notifications to all connected clients */

    /*
    unsigned long t = millis();
    if(t >= ble_loop_timer)
    {
        ble_loop_timer = t + 2000;
        if(pServer->getConnectedCount()) {
            NimBLEService* pSvc = pServer->getServiceByUUID("BAAD");
            if(pSvc) {
                NimBLECharacteristic* pChr = pSvc->getCharacteristic("F00D");
                if(pChr) {
                    pChr->notify(true);
                }
            }
        }    
    }
    */
}


class BLEInput {

    char serialCommandBuffer[MAX_BLE_COMMAND_SIZE] = {0};      // Setup command buffer
    char* serialCommandCurrentByte = serialCommandBuffer;         // current byte being received from serial
    void (*onCommandReceived)(char* command) = nullptr;           // Command received
 
public:

    BLEInput(void (*commandReceivedCallback)(char* command)) {
        this->onCommandReceived = commandReceivedCallback;

        startBleSetup();

    }

    void processSerialCommand() {
        *serialCommandCurrentByte = 0;
        if(onCommandReceived != nullptr)
            onCommandReceived(serialCommandBuffer);
    }

    void loop() {

        //TODO?

    }
};