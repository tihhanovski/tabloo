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

#include <Arduino.h>
#include <ArduinoHttpClient.h> //https://github.com/vshymanskyy/TinyGSM/blob/master/examples/HttpsClient/HttpsClient.ino
#include <TablooGeneral.h>

const char apn[]      = "internet.tele2.ee"; // APN (example: internet.vodafone.pt) use https://wiki.apnchanger.org
const char gprsUser[] = ""; // GPRS User
const char gprsPass[] = ""; // GPRS Password

// SIM card PIN (leave empty, if not defined)
const char simPIN[]   = "0000";

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
    SerialMon.println("Initializing modem...");
    modem.restart();
    // use modem.init() if you don't need the complete restart

    String modemInfo = modem.getModemInfo();
    SerialMon.print("Modem Info: ");
    SerialMon.println(modemInfo);

    // Unlock your SIM card with a PIN if needed
    if (strlen(simPIN) && modem.getSimStatus() != 3 ) {
      modem.simUnlock(simPIN);
    }  
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
        SerialMon.println("Requesting current network time");
        if (modem.getNetworkTime(&year3, &month3, &day3, 
            &hour3, &min3, &sec3,
            &timezone)) 
        {
            SerialMon.print("Year:");
            SerialMon.print(year3);
            SerialMon.print("\tMonth:");
            SerialMon.print(month3);
            SerialMon.print("\tDay:");
            SerialMon.print(day3);
            SerialMon.println("Hour:");
            SerialMon.print(hour3);
            SerialMon.print("\tMinute:");
            SerialMon.print(min3);
            SerialMon.print("\tSecond:");
            SerialMon.println(sec3);
            SerialMon.print("Timezone:");
            SerialMon.println(timezone);
            ret.hours = hour3;
            ret.minutes = min3;
            ret.seconds = sec3;
            break;
        } else {
            SerialMon.print("Couldn't get network time, retrying in 15s.");
            delay(15000L);
        }
    }
    return ret;
}

boolean ensureConnected() {
    if (!modem.isNetworkConnected()) {
        SerialMon.println("Network disconnected");
        if (!modem.waitForNetwork(180000L, true)) {
            SerialMon.println(" fail");
            delay(10000);
            return false;
        }
        if (modem.isNetworkConnected()) {
            SerialMon.println("Network connected");
        }
    }

    if (!modem.isGprsConnected()) {
        SerialMon.println("GPRS disconnected!");
        SerialMon.print(F("Connecting to "));
        SerialMon.print(apn);
        if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
            SerialMon.println(" fail");
            delay(10000);
            return false;
        }
        if (modem.isGprsConnected()) { 
            SerialMon.println("GPRS connected"); 
        }
    }
    return true;
}

boolean connectToNetwork() {
    return ensureConnected();
}