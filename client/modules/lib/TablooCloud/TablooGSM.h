/**
 * Tabloo - opensource bus stop display
 * Connection code for GSM 
 * 
 * @author Ilja Tihhanovski <ilja.tihhanovski@gmail.com>
 *
 * Based on example from TinyGSM by Volodymyr Shymanskyy 
 * https://github.com/vshymanskyy/TinyGSM/blob/master/examples/HttpsClient/HttpsClient.ino 
 *
 */

#ifndef _TABLOOGSM_H_
#define _TABLOOGSM_H_

#include <Arduino.h>
#include <ArduinoHttpClient.h>  //see https://github.com/vshymanskyy/TinyGSM/blob/master/examples/HttpsClient/HttpsClient.ino
#include <TablooGeneral.h>      //Various utilities
#include <TablooSetup.h>        //Settings storage

//All setup moved to preferences, see TablooSetup.h
//const char apn[]      = "internet.tele2.ee"; // APN (example: internet.vodafone.pt) use https://wiki.apnchanger.org
//const char gprsUser[] = ""; // GPRS User
//const char gprsPass[] = ""; // GPRS Password
// SIM card PIN (leave empty, if not defined)
//const char simPIN[]   = "0000";

// TTGO T-Call pins
#define MODEM_RST            5
#define MODEM_PWKEY          4
#define MODEM_POWER_ON       23
#define MODEM_TX             27
#define MODEM_RX             26
#define I2C_SDA              21
#define I2C_SCL              22

// Set serial for debug console (to Serial Monitor, default speed 115200)
#define SerialMon Serial
// Set serial for AT commands (to SIM800 module)
#define SerialAT Serial1

// Configure TinyGSM library
#define TINY_GSM_MODEM_SIM800      // Modem is SIM800
#define TINY_GSM_RX_BUFFER   1024  // Set RX buffer to 1Kb

#include <Wire.h>
#include <TinyGsmClient.h>

#ifdef DUMP_AT_COMMANDS
  #include <StreamDebugger.h>
  StreamDebugger debugger(SerialAT, SerialMon);
  TinyGsm modem(debugger);
#else
  TinyGsm modem(SerialAT);
#endif


// I2C for SIM800 (to keep it running when powered from battery)
TwoWire I2CPower = TwoWire(0);

// TinyGSM Client for Internet connection
TinyGsmClientSecure client(modem);

#define IP5306_ADDR          0x75
#define IP5306_REG_SYS_CTL0  0x00

bool setPowerBoostKeepOn(int en){
    I2CPower.beginTransmission(IP5306_ADDR);
    I2CPower.write(IP5306_REG_SYS_CTL0);
    if (en) {
        I2CPower.write(0x37); // Set bit1: 1 enable 0 disable boost keep on
    } else {
        I2CPower.write(0x35); // 0x37 is default reg value
    }
    return I2CPower.endTransmission() == 0;
}

void startModem() {
    // Start I2C communication
    //I2CPower.begin(I2C_SDA, I2C_SCL, 400);
    //I2CPower.begin(I2C_SDA, I2C_SCL, 400000);

    // Keep power when running from battery
    //bool isOk = setPowerBoostKeepOn(1);
    //SerialMon.println(String("IP5306 KeepOn ") + (isOk ? "OK" : "FAIL"));

    // Set modem reset, enable, power pins
    pinMode(MODEM_PWKEY, OUTPUT);
    pinMode(MODEM_RST, OUTPUT);
    pinMode(MODEM_POWER_ON, OUTPUT);
    digitalWrite(MODEM_PWKEY, LOW);
    digitalWrite(MODEM_RST, HIGH);
    digitalWrite(MODEM_POWER_ON, HIGH);

    // Set GSM module baud rate and UART pins
    SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
    delay(3000);

    // Restart SIM800 module, it takes quite some time
    // To skip it, call init() instead of restart()
    log_i("Initializing modem...");
    modem.restart();
    // use modem.init() if you don't need the complete restart

    #if CORE_DEBUG_LEVEL == 5
    String modemInfo = modem.getModemInfo();
    log_v("Modem info: %s", modemInfo);
    #endif

    char* simPIN = setup_readStringValue(SETUP_KEY_GSM_PIN);
    log_v("SIM PIN: %s", simPIN);

    // Unlock your SIM card with a PIN if needed
    if (strlen(simPIN) && modem.getSimStatus() != 3 ) {
        log_v("Unlocking SIM with PIN");
        modem.simUnlock(simPIN);
    } else
        log_v("No need to unlock SIM");
}

SimpleTime requestNetworkTime() {
    int   year3    = 0;
    int   month3   = 0;
    int   day3     = 0;
    int   hour3    = 0;
    int   min3     = 0;
    int   sec3     = 0;
    float timezone = 0;

    SimpleTime ret;
    for (int8_t i = 5; i; i--) {
        log_v("Requesting current network time");
        if (modem.getNetworkTime(&year3, &month3, &day3, 
            &hour3, &min3, &sec3,
            &timezone)) 
        {
            log_v("Year: %d-%d-%d %d:%d:%d %.1f", year3, month3, day3, hour3, min3, sec3, timezone);

            ret.hours = hour3;
            ret.minutes = min3;
            ret.seconds = sec3;
            break;
        } else {
            log_w("Couldn't get network time (try %d), retrying in 15s.", i);
            //SerialMon.print("Couldn't get network time, retrying in 15s.");
            delay(15000L);
        }
    }
    return ret;
}

boolean ensureConnected() {
    if (!modem.isNetworkConnected()) {
        log_i("Network disconnected");
        if (!modem.waitForNetwork(180000L, true)) {
            log_w("Network connection failed");
            delay(10000);
            return false;
        }
        if (modem.isNetworkConnected()) {
            log_i("Network connected");
            log_i("Signal quality: %d", modem.getSignalQuality());
        }
    }

    if (!modem.isGprsConnected()) {
        bool success  = false;
        char* apn = setup_getStringValue(SETUP_KEY_GSM_APN);
        char* gprsUser = setup_getStringValue(SETUP_KEY_GSM_USER);
        char* gprsPass = setup_getStringValue(SETUP_KEY_GSM_PASS);
        if(strlen(apn)) {
            log_i("GPRS disconnected, connecting to %s as '%s' : '%s'", apn, gprsUser, gprsPass);
            success = modem.gprsConnect(apn, gprsUser, gprsPass);
        }
        else
            log_e("No APN provided. cant connect to GPRS");
        delete apn;
        delete gprsUser;
        delete gprsPass;

        if (!success) {
            log_w("GPRS disconnection failed");
            delay(10000);
            return false;
        }
        if (modem.isGprsConnected()) { 
            log_i("GPRS connected"); 
            log_i("Signal quality: %d", modem.getSignalQuality());
        }
    }
    return true;
}

boolean connectToNetwork() {
    return ensureConnected();
}

#endif