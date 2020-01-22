#include <Color.h>
#include <ESP8266WiFi.h>
//#include <credentials.h>
extern "C" {
#include "user_interface.h"
#include <espnow.h>
}

#define TEST

// Color Use Cases
#define POWER 0
#define INIT 1
#define CONN 2
#define RECV 3
#define WAIT 4
#define SEND 5
#define ERR 6
#define OUT 7

#define WIFI_CHANNEL 1
uint8_t gameserver_ap_mac[]   = {0x5E, 0xCF, 0x7F, 0xF0, 0x1F, 0x9F};
uint8_t gameserver_sta_mac[]  = {0x5C, 0xCF, 0x7F, 0xF0, 0x1F, 0x9F};

// keep in sync with ESP_NOW sensor struct
struct __attribute__((packed)) SENSOR_DATA {
  char data[10];
} sensorData;

// is connected
volatile boolean haveReading = false;

Color LED(12, 13, 15);

#define BASE_SSID "LG_"
#define PSK  "TestTest!"

uint8_t targetsFound = 0;
// 40 macs possible to connect
uint8_t targetMacs[40][6];
uint8_t currentTarget = 0;

//-------------------------------------------------------------------------------------------------------------------------
void setup() {
  changeGPIOstatus(INIT);
  Serial.begin(115200);
#ifdef TEST
  Serial.println("===========================================================");
  Serial.print("Laser Game ESP Now: ");
  Serial.println("Debugmode on");
  Serial.println("===========================================================" + '\n');
#endif
#ifdef TEST
  Serial.println("Gameserver" + '\n');
  //randomSeed(analogRead(0));
#endif
#ifdef TEST
  Serial.println("===========================================================");
  Serial.println("This node AP mac: "); Serial.println(WiFi.softAPmacAddress());
  Serial.println("This node STA mac: "); Serial.println(WiFi.macAddress());
  Serial.println("===========================================================" + '\n');
#endif
  initEspNow();
#ifdef TEST
  Serial.println("Setup done" + '\n');
#endif
}
//-------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------
void loop() {
#ifdef TEST
  Serial.println("loop is starting" + '\n');
#endif
  uint8_t state = 0;
  uint8_t i = 0;
  long sendTime = 0;
  long startTime = 0;
  uint16_t hitCounter = 0;
  long gameLength = 60 * 1000;
  uint16_t currentTime = 0;
  uint8_t bs[sizeof(sensorData)];
  // copy all data from bs to sensorData
  memcpy(bs, &sensorData, sizeof(sensorData));

  while (true) {
    delay(1);
    switch (state) {
      case 0:
        targetsFound = 0;
        scanForTargets();
        state++;
        break;

      case 1:
        if (targetsFound) {
          state++;
          hitCounter = 0;
          for (i = 0; i < targetsFound; i++) {
            changeGPIOstatus(CONN);
            delay(200);
            changeGPIOstatus(OUT);
            delay(800);
#ifdef TEST
            Serial.print("Master found: ");
            Serial.print(i);
            Serial.print(" : ");
            Serial.print(targetsFound);
            Serial.println(" Targets." + '\n');
#endif
          }
          startTime = millis();
          haveReading = false;
        } else {
          delay(100);
          state = 0;
        }
        break;

      case 2:
        currentTarget = random(targetsFound);
        currentTime = random(1500, 7000);
        // current time get splitt up in bs0 and bs1
        bs[0] = currentTime >> 8;
        bs[1] = currentTime & 0xFF;
#ifdef TEST
        Serial.println("Target: ");
        Serial.print(currentTarget);
        Serial.print(" ms: ");
        Serial.println(currentTime + '\n');
#endif
        changeGPIOstatus(SEND);
        // send bs to the choosen target
        esp_now_send(targetMacs[currentTarget], bs, sizeof(sensorData));

        // time sending start time
        sendTime = millis();
        state++;
        changeGPIOstatus(WAIT);
        break;

      case 3:
        if (haveReading) {
          changeGPIOstatus(RECV);
          haveReading = false;
          //target was hit
          if (sensorData.data[0] == 1) {
#ifdef TEST
            Serial.println("Target reported a hit" + '\n');
#endif
            hitCounter++;

            //target timeout
          } else if (sensorData.data[0] == 2) {
#ifdef TEST
            Serial.println("Target reported a timeout" + '\n');
#endif
            changeGPIOstatus(ERR);
          }
          state = 4;
        }
        // random timeout timer -> current time is random
        else if ((millis() - sendTime) > currentTime + 100) {
#ifdef TEST
          Serial.println("Target communication timeout" + '\n');
#endif
          changeGPIOstatus(ERR);
          state = 4;
        }
        Serial.flush();
        if (millis() - startTime > gameLength) {
#ifdef TEST
          Serial.println("Game over");
          Serial.print(hitCounter);
          Serial.println(" hits" + '\n');
#endif
          // blink minimal 10 times
          for (i = 0; i <= hitCounter || i < 10; i++) {
            changeGPIOstatus(RECV);
            delay(200);
            changeGPIOstatus(OUT);
            delay(800);
          }

          delay(10000);
          state = 0;
        }
        break;

      case 4:
        delay(2000);
        state = 2;
        break;
    }
  }
}
//-------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------
void initEspNow() {
  String ssid = String(BASE_SSID) + WiFi.softAPmacAddress().substring(9);
  WiFi.softAP(ssid.c_str(), PSK, WIFI_CHANNEL, 0, 0);
  WiFi.disconnect();
  if (esp_now_init() != 0) {
#ifdef TEST
    Serial.println("*** ESP_Now init failed");
#endif
    ESP.restart();
  }
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_recv_cb([](uint8_t *mac, uint8_t *data, uint8_t len) {
    memcpy(&sensorData, data, sizeof(sensorData));
    if ((mac[0] == targetMacs[currentTarget][0]) && (mac[1] == targetMacs[currentTarget][1]) && (mac[2] == targetMacs[currentTarget][2]) && (mac[3] == targetMacs[currentTarget][3]) && (mac[4] == targetMacs[currentTarget][4]) && (mac[5] == targetMacs[currentTarget][5])) {
      haveReading = true;
    }
  });
}
//-------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------
void scanForTargets() {
  int8_t scanResults = WiFi.scanNetworks();
#ifdef TEST
  Serial.println("\n Scan for targets");
#endif
  if (scanResults == 0) {
#ifdef TEST
    Serial.println("No WiFi devices in AP Mode found");
#endif
    changeGPIOstatus(ERR);
  } else {
#ifdef TEST
    Serial.print("Found ");
    Serial.print(scanResults);
    Serial.println(" devices ");
#endif
    for (int i = 0; i < scanResults; ++i) {
      // Print SSID and RSSI for each device found
      String SSID = WiFi.SSID(i);
      int32_t RSSI = WiFi.RSSI(i);
      String BSSIDstr = WiFi.BSSIDstr(i);
      // SSID starts with LG_
      if (SSID.indexOf("LG_") == 0) {
#ifdef TEST
        Serial.println("===============================================");
        Serial.print("Target :");
        Serial.print(i);
        Serial.print(" : ");
        Serial.println(targetsFound);
        Serial.print("SSID : ");
        Serial.print(i);
        Serial.print(" : ");
        Serial.println(SSID);
        Serial.print("RSSI :");
        Serial.print(i);
        Serial.print(" : ");
        Serial.println(RSSI);
        Serial.print("BSSIDstr :");
        Serial.print(i);
        Serial.print(" : ");
        Serial.println(BSSIDstr);
        Serial.println("===============================================");
#endif
        // Get BSSID => Mac Address of the Slave
        int mac[6];
        if ( 6 == sscanf(BSSIDstr.c_str(), "%x:%x:%x:%x:%x:%x%c",
                         &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5] ) ) {
          for (int ii = 0; ii < 6; ++ii ) {
            targetMacs[targetsFound][ii] = (uint8_t) mac[ii];
          }
        }
        targetsFound++;
      }
    }
  }
  // clean up ram
  WiFi.scanDelete();
}
//-------------------------------------------------------------------------------------------------------------------------
void changeGPIOstatus(uint8_t state) {
  switch (state) {
    // only power is given - default start val
    case 0: LED.yellow();   break;
    // initialisation process is running
    case 1: LED.blue();     break;
    // connection successful
    case 2: LED.white();    break;
    // data received from master/slave
    case 3: LED.green();    break;
    // hit me/wait for hit
    case 4: LED.cyan(); break;
    // got some data to send
    case 5: LED.violette(); break;
    // unexpected case
    default:
    // Error
    case 6: LED.red();      break;
    // turn LED off
    case 7: LED.black();    break;
  }
}
//-------------------------------------------------------------------------------------------------------------------------
