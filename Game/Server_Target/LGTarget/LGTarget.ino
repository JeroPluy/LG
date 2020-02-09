/**
      A game based on ESP8266 – Witty Cloud Modules, ESP-Now, and a laser pointer.
      The goal of the game is to hit the flashing ESPs with the laser gun and achieve a high hit
      rate. It's like Laser Tag on fixed targets with a real laser.
      This is the program for the target ESPs. Please initialize it for all ESPs that you want to
      set as targets.  If you want to play more games at the same time, please change the
      BASE_SSID (line: 48) and the PSK (line: 49).

      @author Jero A
      @version 1.0
      @date 16.01.2020
*/

//___Includes___________________________________________________________________________________________________________________

// for LED control
#include <Color.h>

#include <ESP8266WiFi.h>
extern "C" {
#include "user_interface.h"
#include <espnow.h>
}

//___Modes______________________________________________________________________________________________________________________

// unwanted mode should be commented out
#define DEBUG

//___defines____________________________________________________________________________________________________________________

// Color Use Cases
#define MENU 0      // yellow
#define INIT 1      // blue
#define CONN 2      // white
#define RECV 3      // green
#define WAIT 4      // cyan
#define SEND 5      // violette
#define ERR 6       // red
#define OUT 7


#define WIFI_CHANNEL 1

// define login data
#define BASE_SSID "LG_"
#define PSK  "TestTest!"

//___global_vars________________________________________________________________________________________________________________

// keep in sync with ESP_NOW sensor struct
struct __attribute__((packed)) SENSOR_DATA {
  char data[10];
} sensorData;

// get a msg from other device
volatile boolean haveReading = false;

// should the sensor still be initialized
uint8_t targetAction = 0;

// set LED Pins for rot,green,blue
Color LED(15, 12, 13);

// server esp mac addresses for the targets

uint8_t GAMESERVER_ap_mac[]   = {0xEE, 0xFA, 0xBC, 0x4C, 0x57, 0x12};
uint8_t GAMESERVER_sta_mac[]  = {0xEC, 0xFA, 0xBC, 0x4C, 0x57, 0x12};

// init sensor val
int initVal;

// initSuccess
volatile boolean initSuccess = false;

//___functions_setup____________________________________________________________________________________________________________

void setup() {

  changeGPIOstatus(INIT);

#ifdef DEBUG
  Serial.begin(74880);
  Serial.println("===========================================================");
  Serial.print("Laser Game: ");
  Serial.println("Debugmode on");
  Serial.println("===========================================================");
  Serial.println('\n');


  Serial.println("===========================================================");
  Serial.println("Target" );
  Serial.println("-----------------------------------------------------------");
  Serial.print("This node AP mac: "); Serial.println(WiFi.softAPmacAddress());
  Serial.print("This node STA mac: "); Serial.println(WiFi.macAddress());
  Serial.println("===========================================================");
  Serial.println('\n');
#endif

  initEspNow();


#ifdef DEBUG
  Serial.println("===========================================================");
  Serial.println("Setup done");
  Serial.println("-----------------------------------------------------------");
  Serial.println("loop is starting");
  Serial.println("===========================================================");
  Serial.println('\n');
#endif
}


//___target_loop________________________________________________________________________________________________________________

void loop() {

  // activ timer for connection
  uint16_t activeTime = 0;

  long startTime = 0;

  // marks the end of the intern loop
  boolean endFlag = false;

  // connection array
  uint8_t bs[sizeof(sensorData)];

  // value of the LDR before hit while game
  int analogVal = 0;

  //----------------------------------------------------------------------------------------------------------------------------

  // standard led status if it isn't connected
  if (!initSuccess) {
    changeGPIOstatus(ERR);
  }
#ifdef DEBUG
  else if (1) {
    changeGPIOstatus(RECV);
  }
#endif

  else {
    changeGPIOstatus(OUT);
  }

  // if the target gets a message from the server
  if (haveReading) {

    // reset the message notification
    haveReading = false;

    // get the target action
    targetAction = sensorData.data[3];

    //time for the hit
    activeTime = (sensorData.data[0] << 8) | sensorData.data[1];



#ifdef DEBUG
    Serial.println("===========================================================");
    Serial.print("activated for ");
    Serial.print(activeTime);
    Serial.println(" ms");
    Serial.print("Sensor initialization : ");
    Serial.println(initVal);
    Serial.println("===========================================================");
    Serial.println('\n');
#endif

    // start time for the endFlag loop
    startTime = millis();

    if (targetAction == INIT) {

      // init Sensor
      initVal = initSensor();


      if ((initVal < 5) || (initVal > 850)) {
        // message for the server
        bs[0] = 4;
        changeGPIOstatus(ERR);
      } else {
        bs[0] = 3;
        bs[1] = initVal >> 8;
        bs[2] = initVal & 0xFF;
        initSuccess = true;

#ifdef DEBUG
        Serial.println("===========================================================");
        Serial.print("init VAL: ");
        Serial.println(initVal);
        Serial.print("120% VAL: ");
        Serial.println(initVal * 1.2);
        Serial.println("===========================================================");
        Serial.println('\n');
#endif

      }

      //------------------------------------------------------------------------------------------------------------------------

    } else {

      if (targetAction == MENU) {
        // if target is the target to start the game
        changeGPIOstatus(RECV);

      } else {

        changeGPIOstatus(WAIT);
      }

      // accessable as target (inner loop)
      while (endFlag == false) {
        // wathcdog
        delay(1);
        // get the actuell value of the light sensor
        analogVal = analogRead(A0);

        // whats happen next:

        //if needs to long to answer -> timeout
        if (millis() - startTime > activeTime) {

          // message for the server
          bs[0] = 2;
          // stop endFlag loop
          endFlag = true;
          changeGPIOstatus(SEND);
#ifdef DEBUG
          Serial.println("===========================================================");
          Serial.println("Target timeout");
          Serial.println("===========================================================");
          Serial.println('\n');
#endif
          break;
        }

        /*
          #ifdef DEBUG
                //simulate hits for long active times
                if (millis() - startTime > 4000) {
                  // write a hit in the connection array
                  bs[0] = 1;
                  endFlag = true;
                  Serial.println("===========================================================");
                  Serial.println("HIT");
                  Serial.println("===========================================================");
                  Serial.println('\n');
                }
          #endif
        */

        // normal hit
        if (analogVal >= (initVal * 1.2)) {
#ifdef DEBUG

          Serial.println("===========================================================");
          Serial.println("HIT");
          Serial.println("-----------------------------------------------------------");
          Serial.print("Ziel getroffen, VAL: ");
          Serial.println(analogVal);
          Serial.println("===========================================================");
          Serial.println('\n');
#endif
          // write a hit in the connection array
          bs[0] = 1;
          endFlag = true;
          changeGPIOstatus(RECV);
        }
      }
    }

    //--------------------------------------------------------------------------------------------------------------------------

    //send the message
    esp_now_send(GAMESERVER_ap_mac, bs, sizeof(sensorData));

  }
}


//___extra_functions____________________________________________________________________________________________________________


void initEspNow() {
#ifdef DEBUG
  Serial.println("===========================================================");
  Serial.println("init methode");
  Serial.println("===========================================================");
  Serial.println('\n');
#endif

  // get ssid of the device
  String ssid = String(BASE_SSID) + WiFi.softAPmacAddress().substring(9);
  WiFi.softAP(ssid.c_str(), PSK, WIFI_CHANNEL, 0, 0);
  WiFi.disconnect();

  // if initialization fails
  if (esp_now_init() != 0) {
#ifdef DEBUG
    Serial.println("===========================================================");
    Serial.println(" ESP_Now init failed");
    Serial.println("===========================================================");
    Serial.println('\n');
#endif
    changeGPIOstatus(ERR);
    delay(3000);
    ESP.restart();
  }

  //choose the role that to be running with
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);

  esp_now_add_peer(GAMESERVER_ap_mac, ESP_NOW_ROLE_COMBO, WIFI_CHANNEL, NULL, 0);
  esp_now_add_peer(GAMESERVER_sta_mac, ESP_NOW_ROLE_COMBO, WIFI_CHANNEL, NULL, 0);
#ifdef DEBUG
  Serial.println("===========================================================");
  Serial.print("Add Server peer : ");
  Serial.print(" (MAC: ");
  for (int i = 0; i < 6; i++) {
    Serial.print(GAMESERVER_ap_mac[i], HEX);
    if (i < 5) {
      Serial.print(":");
    } else {
      Serial.println(")");
    }
  }
  Serial.println("===========================================================");
  Serial.println('\n');
#endif


  // triggers everytime something is received by a device
  esp_now_register_recv_cb([](uint8_t *mac, uint8_t *data, uint8_t len) {
    // copy the data of the transfer array to data
    memcpy(&sensorData, data, sizeof(sensorData));
    haveReading = true;
  });
  
}


//___initSensor_________________________________________________________________________________________________________________

int initSensor() {

  changeGPIOstatus(INIT);

  int initVal = 0;

  // get four current vals befor the game starts to calibrate itself
  delay(100);
  int val1 = analogRead(A0);
  delay(800);
  int val2 = analogRead(A0);
  delay(800);
  int val3 = analogRead(A0);
  delay(800);
  int val4 = analogRead(A0);

#ifdef DEBUG
  Serial.println("===========================================================");
  Serial.print("First Val: ");
  Serial.println(val1);
  Serial.print("Second Val: ");
  Serial.println(val2);
  Serial.print("Third Val: ");
  Serial.println(val3);
  Serial.print("Fourth Val: ");
  Serial.println(val4);
  Serial.println("-----------------------------------------------------------");
#endif

  initVal = (val1 + val2 + val3 + val4) / 4;
#ifdef DEBUG
  Serial.print("calc initVal Val: ");
  Serial.println(initVal);
  Serial.println("===========================================================");
  Serial.println('\n');
#endif
  return initVal;
}

//___changeGPIOstatus___________________________________________________________________________________________________________

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

//______________________________________________________________________________________________________________________________
