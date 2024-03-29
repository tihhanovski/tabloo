// Settings functions for tabloo project
#ifndef _TABLOOSETUP_H_
#define _TABLOOSETUP_H_

#include <Arduino.h>
#include <Preferences.h>
#include <SerialInput.h>

Preferences preferences;

#ifndef SETUP_LED_PIN
#define SETUP_LED_PIN 2
#endif
#ifndef SETUP_BUTTON_PIN
#define SETUP_BUTTON_PIN 4
#endif
#ifndef SETUP_BLINK_PAUSE
#define SETUP_BLINK_PAUSE 500
#endif
#ifndef SETUP_TIMEOUT_PAUSE
#define SETUP_TIMEOUT_PAUSE 30000
#endif

#ifndef SETUP_BLE_ENABLED
#define SETUP_BLE_ENABLED false
#endif

#ifndef SETUP_NAME
#define SETUP_NAME "tabloo"
#endif

const char *CMD_SETUP_RESET = "setup reset"; // Comand to reset setup
const char *CMD_SETUP_SAVE = "setup save"; // Comand to save setup
const char *CMD_SETUP_SET = "setup set ";  // Command to set setup variable
const size_t CMD_SETUP_SET_LEN = strlen(CMD_SETUP_SET);
const char *CMD_SETUP_GET = "setup get ";  // Command to set setup variable
const size_t CMD_SETUP_GET_LEN = strlen(CMD_SETUP_GET);
const char *CMD_SETUP_LIST = "setup list"; // Command to show setup variables and their values

const char* SETUP_KEY_GSM_APN = "apn";
const char* SETUP_KEY_GSM_PIN = "pin";
const char* SETUP_KEY_GSM_USER = "gsm_user";
const char* SETUP_KEY_GSM_PASS = "gsm_pass";
const char* SETUP_KEY_STOP_CODE = "stop";

const char* SETUP_KEY_WIFI_SSID = "ssid";
const char* SETUP_KEY_WIFI_PASS = "pass";



const uint8_t SETUP_POSSIBLE_KEYS_COUNT = 7;
const char *SETUP_POSSIBLE_KEYS[] = {
    SETUP_KEY_GSM_APN, 
    SETUP_KEY_GSM_PIN, 
    SETUP_KEY_GSM_USER, 
    SETUP_KEY_GSM_PASS,
    SETUP_KEY_STOP_CODE,
    SETUP_KEY_WIFI_SSID,
    SETUP_KEY_WIFI_PASS
};


#if SETUP_BLE_ENABLED
#include <TablooBLESetup.h>
#endif


boolean setupState = false;
boolean setupLedOn = false;
unsigned long setupBlinkTimer = 0;
unsigned long setupIdleTimer = 0;

void resetSetupIdleTimer()
{
    setupIdleTimer = millis() + SETUP_TIMEOUT_PAUSE;
}

void blinkSetupLed()
{
    boolean newLedState;
    if (setupState)
        newLedState = !setupLedOn;
    else
        newLedState = false;
    if (newLedState != setupLedOn)
    {
        setupLedOn = newLedState;
        digitalWrite(SETUP_LED_PIN, setupLedOn);
    }
}


char setupValue[256] = {0};

char* setup_readStringValue(const char* key) {
    size_t l = preferences.getString(key, setupValue, 255);
    setupValue[l] = 0;
    return setupValue;
}

char* setup_getStringValue(const char* key) {
    size_t l = preferences.getString(key, setupValue, 255);
    setupValue[l] = 0;
    char* ret = new char[l + 1];
    memcpy(ret, setupValue, l + 1);
    return ret;
}

void setup_putStringValue(const char* key, const char* val) {
    log_v("key='%s', val='%s'", key, val);
    if (strlen(key)) {
        if(strlen(val))
            preferences.putString(key, val);
        else
            preferences.remove(key);
    } else
        log_e("Arguments not given");
}


/**
 * Process command received
 * @param command c string
 */
void processCommand(char *command)
{
    if(!setupState) return;
    resetSetupIdleTimer();

    char *arg1;
    char *arg2;

    log_i("Received command '%s'", command);

    if (!strcmp(CMD_SETUP_LIST, command)) {
        Serial.println("Setup list:");
        for(uint8_t i = 0; i < SETUP_POSSIBLE_KEYS_COUNT; i++) {
            const char* key = SETUP_POSSIBLE_KEYS[i];
            Serial.printf("\t%s = ", key);
            PreferenceType t = preferences.getType(key);
            if(t == PT_INVALID)
                Serial.print("**UNDEFINED**");
            else
            {
                char* value = setup_readStringValue(key);
                Serial.printf("'%s'", value);
            }
            Serial.println("");
        }
    }

    if (!strcmp(CMD_SETUP_SAVE, command))
        log_e("command %s is not implemented", command);

    if (!strcmp(CMD_SETUP_RESET, command)) {
        log_i("Reset setup");
        preferences.clear();
    }

    if (!memcmp(CMD_SETUP_SET, command, CMD_SETUP_SET_LEN))
    {
        Serial.print("\tSetup set '");
        arg1 = command + CMD_SETUP_SET_LEN;
        arg2 = arg1;
        while (*arg2 != 0)
        {
            if (*arg2 == '=')
            {
                *arg2 = 0;
                arg2++;
                break;
            }
            arg2++;
        }
        Serial.print(arg1);
        Serial.print("' to '");
        Serial.print(arg2);
        Serial.println("'");
        
        setup_putStringValue(arg1, arg2);
    }


    if(!memcmp(CMD_SETUP_GET, command, CMD_SETUP_GET_LEN)) {
        arg1 = command + CMD_SETUP_GET_LEN;
        log_i("setup get %s", arg1);
        PreferenceType t = preferences.getType(arg1);
        if(t == PT_INVALID)
            log_e("nothing saved for '%s'", arg1);
        else
            log_i("retrieved from preferencies: '%s'", setup_readStringValue(arg1));
    }
}

SerialInput serialInput(processCommand);

#if SETUP_BLE_ENABLED
TablooBLESetup bleInput(processCommand, setup_getStringValue, setup_putStringValue);
#endif

void setup_start()
{
    log_i("Setup button pin: %d, LED pin: %d", SETUP_BUTTON_PIN, SETUP_LED_PIN);
    log_i("Preferencies name: '%s'", SETUP_NAME);

    pinMode(SETUP_BUTTON_PIN, INPUT_PULLUP);
    pinMode(SETUP_LED_PIN, OUTPUT);
    preferences.begin(SETUP_NAME, false);

    #if SETUP_BLE_ENABLED
    bleInput.init();
    #endif
}

void setup_loop()
{
    unsigned long t = millis();
    if(digitalRead(SETUP_BUTTON_PIN) == LOW) {
        if (!setupState)
        {
            log_v("Enter setup mode");
            setupState = true;
            #if SETUP_BLE_ENABLED
            bleInput.start();
            #endif
            resetSetupIdleTimer();
        }
    }

    if (setupState)
    {
        serialInput.loop();

        if (t > setupIdleTimer)
        {
            log_v("Timeout. Exit setup mode");
            setupState = false;
            digitalWrite(SETUP_LED_PIN, 0);
            #if SETUP_BLE_ENABLED
            bleInput.stop();
            #endif
        }

        if (t > setupBlinkTimer)
        {
            blinkSetupLed();
            setupBlinkTimer = t + SETUP_BLINK_PAUSE;
        }
    }
}

#endif