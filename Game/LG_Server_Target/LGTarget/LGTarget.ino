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
  Serial.println("Target" + '\n');
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
  uint16_t waitTime = 0;
  long startTime = 0;
  // traget was hit
  boolean endFlag = false;
  // connection array
  uint8_t bs[sizeof(sensorData)];
  // value of the LDR before hit
  int initVal = 0;
  // value while game
  int analogVal = 0;
  // if connected
  if (haveReading) {
    initVal = initSensor();
    if (initVal < 5) {
      changeGPIOstatus(ERR);
    } else {
      haveReading = false;
      // wait time == current time
      waitTime = (sensorData.data[0] << 8) | sensorData.data[1];
#ifdef TEST
      Serial.print("activated for ");
      Serial.print(waitTime);
      Serial.println(" ms");
#endif
      startTime = millis();
      changeGPIOstatus(WAIT);
      // accessable as target
      while (endFlag == false) {
        delay(1);
        // get the actuell value of the light sensor
        analogVal = analogRead(A0);

        // whats happen next:

        //timeout
        if (millis() - startTime > waitTime) {
          // message for the server
          bs[0] = 2;
          endFlag = true;
#ifdef TEST
          Serial.println("timeout");
#endif
        }

#ifdef TEST
        //simulate hits for long times on
        if (millis() - startTime > 4000) { //simulate hits for long times on
          bs[0] = 1;
          endFlag = true;
          Serial.println("hit");
        }
#endif

        // hit
        if (analogVal >= (initVal * 1.2)) {
#ifdef TEST
          Serial.print("Licht getroffen, VAL: ");
          Serial.println(analogVal);
#endif
          // write a hit in the connection array
          bs[0] = 1;
          endFlag = true;
        }
      }
      changeGPIOstatus(SEND);
      esp_now_send(gameserver_ap_mac, bs, sizeof(sensorData));
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
  esp_now_add_peer(gameserver_ap_mac, ESP_NOW_ROLE_COMBO, WIFI_CHANNEL, NULL, 0);
  esp_now_add_peer(gameserver_sta_mac, ESP_NOW_ROLE_COMBO, WIFI_CHANNEL, NULL, 0);
  esp_now_register_recv_cb([](uint8_t *mac, uint8_t *data, uint8_t len) {
    memcpy(&sensorData, data, sizeof(sensorData));
    haveReading = true;
  });
}
//-------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------
int initSensor() {
  changeGPIOstatus(INIT);
  int val1 = analogRead(A0);
  int initVal = 0;
#ifdef TEST
  Serial.print("First Val: ");
  Serial.println(val1);
#endif
  delay(500);
  int val2 = analogRead(A0);
#ifdef TEST
  Serial.print("Second Val: ");
  Serial.println(val2);
#endif
  delay(500);
  int val3 = analogRead(A0);
#ifdef TEST
  Serial.print("Third Val: ");
  Serial.println(val3);
#endif
  delay(500);
  int val4 = analogRead(A0);
#ifdef TEST
  Serial.print("Fourth Val: ");
  Serial.println(val4);
#endif
  initVal = (val1 + val2 + val3 + val4) / 4;
#ifdef TEST
  Serial.print("calc init Val: ");
  Serial.println(initVal);
#endif
  if (initVal >= 1000) {
    return -1;
  }
  return initVal;
}
//-------------------------------------------------------------------------------------------------------------------------
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
