/**
 * Tabloo - opensource bus stop display
 * Setup over BLE
 * @author Ilja Tihhanovski <ilja.tihhanovski@gmail.com> 
 */

#ifndef _TABLOOBLESETUP_H_
#define _TABLOOBLESETUP_H_

#define TABLOO_BLE_NAME "Tabloo_v1"
#define TABLOO_BLE_SERVICE_UUID "cc00"  //"AAAA"
#define TABLOO_BLE_CHAR_APN_UUID "cc01" //"93a1cb91-0bc1-4e4f-87a4-090843cbc1f1"
#define TABLOO_BLE_CHAR_GSM_PIN_UUID "cc02"

#include <Arduino.h>
#include <NimBLEDevice.h>
#define MAX_BLE_COMMAND_SIZE 100

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

class CharacteristicCallbacks: public NimBLECharacteristicCallbacks {

    void (*ble_onCommandReceived)(char* command) = nullptr;           // Command received
    char* (*readSetupProperty)(const char* key) = nullptr;
    void (*writeSetupProperty)(const char* key, const char* value) = nullptr;

    void onRead(NimBLECharacteristic* pCharacteristic){
        log_i("'%s' --> '%s'", pCharacteristic->getUUID().toString().c_str(), pCharacteristic->getValue().c_str());
    };

    void onWrite(NimBLECharacteristic* pCharacteristic) {

        const char* uuid = pCharacteristic->getUUID().toString().c_str() + 2;
        const char* cmd = pCharacteristic->getValue().c_str();
        log_i("'%s' <-- '%s'", uuid, cmd);

        if(writeSetupProperty != nullptr) {
            log_v("will compare '%s' and '%s' --> %i", uuid, TABLOO_BLE_CHAR_APN_UUID, strcmp(uuid, TABLOO_BLE_CHAR_APN_UUID));
            if(!strcmp(uuid, TABLOO_BLE_CHAR_APN_UUID))
                writeSetupProperty(SETUP_KEY_GSM_APN, cmd);
        }

         
        // if(ble_onCommandReceived != nullptr) {
        //     log_i("send command %s", cmd);
        //     ble_onCommandReceived(cmd);
        // }

        // if(!strcmp(CMD_OPEN, cmd))
        //     onOpen();
        // if(!strcmp(CMD_CLOSE, cmd))
        //     onClose();


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

public:

    void setCommandReceivedCallback(void (*commandReceivedCallback)(char* command)) {
        this->ble_onCommandReceived = commandReceivedCallback;
    }

    void setReadSetupProperty(char* (*readSetupProperty)(const char* key)) {
        this->readSetupProperty = readSetupProperty;
    }

    void setWriteSetupProperty(void (*writeSetupProperty)(const char* key, const char* value)) {
        this->writeSetupProperty = writeSetupProperty;
    }

};

/** Handler class for descriptor actions */    
class DescriptorCallbacks : public NimBLEDescriptorCallbacks {
    void onWrite(NimBLEDescriptor* pDescriptor) {
        NimBLEAttValue val = pDescriptor->getValue();
        std::string dscVal((char*)val.data(), pDescriptor->getLength());
        Serial.print("Descriptor written value:");
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
static NimBLEServer* pServer;

class TablooBLESetup {
    // void (*ble_onCommandReceived)(char* command) = nullptr;           // Command received
    NimBLEService* pSetupService;
    char* (*readSetupProperty)(const char* key) = nullptr;
public:

    TablooBLESetup(
        void (*commandReceivedCallback)(char* command),
        char* (*readSetupProperty)(const char* key),
        void (*writeSetupProperty)(const char* key, const char* value)
        ) {
        // this->ble_onCommandReceived = commandReceivedCallback;
        this->readSetupProperty = readSetupProperty;

        chrCallbacks.setCommandReceivedCallback(commandReceivedCallback);
        chrCallbacks.setReadSetupProperty(readSetupProperty);
        chrCallbacks.setWriteSetupProperty(writeSetupProperty);
    }

    void addPropChar(const char* uuid, const char* setupKey) {
        NimBLECharacteristic* chr = pSetupService->createCharacteristic(
                                                uuid,
                                                NIMBLE_PROPERTY::READ |
                                                NIMBLE_PROPERTY::WRITE |
                                                NIMBLE_PROPERTY::NOTIFY
                                                );
        chr->setValue("----"); //readSetupProperty(setupKey)
        chr->setCallbacks(&chrCallbacks);
        log_v("callback set");
}

    void init() {
        
        NimBLEDevice::init(TABLOO_BLE_NAME);
        log_v("initialized");

        NimBLEDevice::setSecurityAuth(/*BLE_SM_PAIR_AUTHREQ_BOND | BLE_SM_PAIR_AUTHREQ_MITM |*/ BLE_SM_PAIR_AUTHREQ_SC);

        pServer = NimBLEDevice::createServer();
        // pServer->setCallbacks(new ServerCallbacks());

        log_v("server created");
    
        pSetupService = pServer->createService(TABLOO_BLE_SERVICE_UUID);
        ///TODO 
        addPropChar(TABLOO_BLE_CHAR_APN_UUID, SETUP_KEY_GSM_APN);
        addPropChar(TABLOO_BLE_CHAR_GSM_PIN_UUID, SETUP_KEY_GSM_PIN);

        pSetupService->start();
        log_v("service started");

        NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
        pAdvertising->addServiceUUID(pSetupService->getUUID());
    }

    void start() {
        Serial.println("Start NimBLE Server");
        NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
        if(!pAdvertising->isAdvertising()) {
            pAdvertising->setScanResponse(true);
            pAdvertising->start();
        }

        log_v("Advertising Started");
    }

    void stop() {
        Serial.println("Stop NimBLE Server");
        NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
        if(pAdvertising->isAdvertising()) {
            pAdvertising->stop();
            pAdvertising->setScanResponse(false);
        }
    }
};

#endif
