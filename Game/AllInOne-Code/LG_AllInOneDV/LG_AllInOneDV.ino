/**
      A game based on ESP8266 – Witty Cloud Moduls, ESP-Now and a laser pointer.
      The goal of the game is to hit the blinking ESPs with the laser gun and reach a high score of hits.
      It's like lasertag on fixed targets with a real laser.
      This is the full code version. You need to comment out the unwanted modes to get a working program.


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

// unwanted modes should be commented out
//#define DEBUG
//#define TARGET
#define GAMESERVER

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

//___target_vars________________________________________________________________________________________________________________

// server esp mac addresses for the targets
#ifdef TARGET

uint8_t GAMESERVER_ap_mac[]   = {0xEE, 0xFA, 0xBC, 0x4C, 0x57, 0x12};
uint8_t GAMESERVER_sta_mac[]  = {0xEC, 0xFA, 0xBC, 0x4C, 0x57, 0x12};

// init sensor val
int initVal;

// initSuccess
volatile boolean initSuccess = false;

//last message time
long lastRequestTime = 0;

#endif

//___gameserver_vars____________________________________________________________________________________________________________

// target administration
#ifdef GAMESERVER

// counter var
uint8_t potentialTargets = 0;
// counter var
uint8_t targetsFound = 0;

// arrayy list for saving target macs
uint8_t potentialTargetsMacs[40][6];
// 40 macs possible to connect
uint8_t targetMacs[40][6];

uint8_t currentTarget = 0;

#endif


//___function_setup_____________________________________________________________________________________________________________

void setup() {

  changeGPIOstatus(INIT);

#ifdef DEBUG
  Serial.begin(74880);
  Serial.println("===========================================================");
  Serial.print("Laser Game: ");
  Serial.println("Debugmode on");
  Serial.println("===========================================================");
  Serial.println('\n');

#ifdef GAMESERVER
  Serial.println("===========================================================");
  Serial.println("Gameserver");
#endif

#ifdef TARGET
  Serial.println("===========================================================");
  Serial.println("Target" );
#endif

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
#ifdef TARGET
  Serial.println("-----------------------------------------------------------");
  Serial.println("loop is starting");
#endif
  Serial.println("===========================================================");
  Serial.println('\n');
#endif
}


//___gameserver_loop____________________________________________________________________________________________________________

void loop() {

#ifdef GAMESERVER
#ifdef DEBUG
  Serial.println("===========================================================");
  Serial.println("loop is starting");
  Serial.println("===========================================================");
  Serial.println('\n');
#endif

  // initialize all vars for the server
  uint8_t state = 0;

  // counter var
  uint8_t i = 0;

  //start tries
  uint8_t startTry = 0;

  // target counter var
  uint8_t nextTarget = 0;

  //send timer var
  long sendTime = 0;

  //process timer var
  long startTime = 0;

  uint16_t hitCounter = 0;

  // timer for timeouts after this time (after 60 sec.)
  long gameLength = 60 * 1000;

  // timer for split times ( correct initialization follows later)
  uint16_t currentTime = 0;

  // transfer array for data
  uint8_t bs[sizeof(sensorData)];

  // copy all data from bs to sensorData (clear sensorData)
  memcpy(bs, &sensorData, sizeof(sensorData));

  //----------------------------------------------------------------------------------------------------------------------------

  while (true) {
    // Need some delay for watchdog feeding in loop
    delay(1);


    switch (state) {

      //------------------------------------------------------------------------------------------------------------------------

      //search for targets
      case 0:
        changeGPIOstatus(INIT);
        targetsFound = 0;
        potentialTargets = 0;
        startTry = 0;
        scanForTargets();
        state++;
        break;

      //------------------------------------------------------------------------------------------------------------------------

      // send potential targets a sensor init request
      case 1:
        if (potentialTargets) {
          currentTarget = nextTarget;
          currentTime = 5000;

          // current time get splitt up in bs0 and bs1
          bs[0] = currentTime >> 8;
          bs[1] = currentTime & 0xFF;

          // transmite the target which target action
          bs[3] = INIT;

#ifdef DEBUG
          Serial.println("===========================================================");
          Serial.print("Connected to Target ");
          Serial.print(currentTarget + 1);        // i + 1 -> just for better reading, first target else would be target 0
          Serial.print(" (MAC: ");
          for (i = 0; i < 6; i++) {
            Serial.print(potentialTargetsMacs[currentTarget][i], HEX);
            if (i < 5) {
              Serial.print(":");
            } else {
              Serial.print(")");
            }
          }
          Serial.print(" for ");
          Serial.print(currentTime);
          Serial.println(" ms.");
          Serial.println("-----------------------------------------------------------");
#endif

          // sends bs to the selected target
          esp_now_send(potentialTargetsMacs[currentTarget], bs, sizeof(sensorData));

          // start sending timer
          sendTime = millis();
          state++;
          haveReading = false;
          changeGPIOstatus(INIT);
        }

        // if no potential targets were found restart searching
        else {
          delay(100);
          state = 0;
        }
        break;

      //------------------------------------------------------------------------------------------------------------------------

      // get the result of the sensor test from the target
      case 2:
        //if gets an answer
        if (haveReading) {

          changeGPIOstatus(RECV);

          // reset the message notification
          haveReading = false;

          //if target responds sensor working
          if (sensorData.data[0] == 3) {

#ifdef DEBUG
            int initVal = (sensorData.data[1] << 8) | sensorData.data[2];
            Serial.print("Target sensor is working fine: ");
            Serial.println(initVal);
            Serial.println("===========================================================");
            Serial.println('\n');
#endif
            // add it to the actuel target list for the game
            for (i = 0; i < 6; i++) {
              targetMacs[targetsFound][i] = potentialTargetsMacs[currentTarget][i];
            }
            targetsFound++;
            state = 3;
          }

          //target respond timeout
          else if (sensorData.data[0] == 4) {
#ifdef DEBUG
            int initVal = (sensorData.data[1] << 8) | sensorData.data[2];
            Serial.println("Target sensor is useless: ");
            Serial.println(initVal);
            Serial.println("===========================================================");
            Serial.println('\n');
#endif
            state = 3;
          }
        }

        //timeout timer for no response
        if ((millis() - sendTime) > currentTime + 100) {
#ifdef DEBUG
          Serial.println("Target communication timeout");
          Serial.println("No answer from the Target");
          Serial.println("===========================================================");
          Serial.println('\n');
#endif
          changeGPIOstatus(ERR);
          state = 3;
        }

        Serial.flush();
        break;

      //------------------------------------------------------------------------------------------------------------------------

      // next Target for sensor test
      case 3:
        // next target
        if ((currentTarget + 1) < potentialTargets) {
          nextTarget++;
          state = 1;
        } else {
#ifdef DEBUG
          Serial.println("===========================================================");
          Serial.println("Target sensor check completed");
          Serial.println("===========================================================");
          Serial.println('\n');
#endif

          // set targetAction off
          targetAction = 0;
          // delete the reference in the array
          bs[3] = 0;
          // reset potentials
          potentialTargets = 0;
          // reset next Targe
          nextTarget = 0;

          state = 4;
        }
        break;

      //------------------------------------------------------------------------------------------------------------------------

      // shows how many targets you found
      case 4:
        if (targetsFound) {

          state++;

          // shows how many targets you found (LED blink)
          for (i = 0; i < targetsFound; i++) {
            changeGPIOstatus(CONN);
            delay(200);
            changeGPIOstatus(OUT);
            delay(800);
#ifdef DEBUG
            Serial.println("===========================================================");
            Serial.print("Server found Target ");
            Serial.print(i + 1);  // i + 1 -> just for better reading, first target else would be target 0
            Serial.print(" of ");
            Serial.print(targetsFound);
            Serial.println(" Target/s.");
            Serial.println("===========================================================");
            Serial.println('\n');
#endif

          }

          // no interesting message received
          haveReading = false;

          // set targetAction off
          targetAction = 0;

        }
        // if no targets were found restart searching
        else {
          delay(100);
          state = 0;
        }
        break;

      //------------------------------------------------------------------------------------------------------------------------

      // start menu, in which the first target of the list is green and the game starts when it is hit
      case 5:

        // set thee first target as start-"button"
        currentTarget = 0;

        // refresh after 1 min.
        currentTime = 60000;

        // current time get splitt up in bs0 and bs1
        bs[0] = currentTime >> 8;
        bs[1] = currentTime & 0xFF;

        // transmite the target which target action
        bs[3] = MENU;

#ifdef DEBUG
        Serial.println("===========================================================");
        Serial.print("Connected to Target ");
        Serial.print(currentTarget + 1);        // i + 1 -> just for better reading, first target else would be target 0
        Serial.print(" (MAC: ");
        for (i = 0; i < 6; i++) {
          Serial.print(targetMacs[currentTarget][i], HEX);
          if (i < 5) {
            Serial.print(":");
          } else {
            Serial.print(")");
          }
        }
        Serial.print(" for ");
        Serial.print(currentTime);
        Serial.println(" ms.");
        Serial.println("Waiting for the game to start.");
        Serial.println("===========================================================");
        Serial.println('\n');
#endif

        // sends bs to the selected target
        esp_now_send(targetMacs[currentTarget], bs, sizeof(sensorData));

        // start sending timer
        sendTime = millis();
        state++;
        changeGPIOstatus(MENU);
        break;

      //------------------------------------------------------------------------------------------------------------------------

      // analyses the answer of the start target
      case 6:

        //if gets an answer
        if (haveReading) {

          changeGPIOstatus(RECV);
          // reset the message notification
          haveReading = false;

          //if target responds hit
          if (sensorData.data[0] == 1) {
#ifdef DEBUG
            Serial.println("===========================================================");
            Serial.println("Game could start");
            Serial.println("===========================================================");
            Serial.println('\n');
#endif
            state = 7;

            // set the target action to game setting
            bs[3] = 3;

            // start time for the game
            startTime = millis();

            // reset hit counter
            hitCounter = 0;

            countDown();
          }
          //target respond no hit
          else if (sensorData.data[0] == 2) {
#ifdef DEBUG
            Serial.println("===========================================================");
            Serial.println("Target report no hit");
            Serial.println("===========================================================");
            Serial.println('\n');

#endif
            startTry++;

            if (startTry == 5) {
              // restart the program
              state = 0;
            } else {

              // refresh the start-"button"
              state = 5;
            }
          }
        }

        //timeout timer for no response
        else if ((millis() - sendTime) > currentTime + 100) {
#ifdef DEBUG
          Serial.println("===========================================================");
          Serial.println("Target communication timeout");
          Serial.println("No answer from the Target");
          Serial.println("===========================================================");
          Serial.println('\n');
#endif

          changeGPIOstatus(ERR);
          delay(2000);

          //restart scan for targets
          state = 0;
          break;
        }

        Serial.flush();
        break;

      //------------------------------------------------------------------------------------------------------------------------

      // selects a random target from the target list and transmits the response time (currentTime)
      // the target knows what to do
      case 7:
        currentTarget = random(targetsFound);

        // the target has 1.5 - 7 seconds to respond a hit
        currentTime = random(1500, 7000);

        // current time get splitt up in bs0 and bs1
        bs[0] = currentTime >> 8;
        bs[1] = currentTime & 0xFF;

#ifdef DEBUG
        Serial.println("===========================================================");
        Serial.print("Connected to Target ");
        Serial.print(currentTarget + 1);        // i + 1 -> just for better reading, first target else would be target 0
        Serial.print(" (MAC: ");
        for (i = 0; i < 6; i++) {
          Serial.print(targetMacs[currentTarget][i], HEX);
          if (i < 5) {
            Serial.print(":");
          } else {
            Serial.print(")");
          }
        }
        Serial.print(" for ");
        Serial.print(currentTime);
        Serial.println(" ms.");
        Serial.println("===========================================================");
        Serial.println('\n');
#endif

        // sends bs to the selected target
        esp_now_send(targetMacs[currentTarget], bs, sizeof(sensorData));

        // start sending timer
        sendTime = millis();
        state++;
        changeGPIOstatus(WAIT);
        break;

      //------------------------------------------------------------------------------------------------------------------------

      // analyses the answer of the target
      case 8:

        //if gets an answer
        if (haveReading) {

          changeGPIOstatus(RECV);
          // reset the message notification
          haveReading = false;

          //if target responds hit
          if (sensorData.data[0] == 1) {
#ifdef DEBUG
            Serial.println("===========================================================");
            Serial.println("Target send HIT 0.o ");
            Serial.println("===========================================================");
            Serial.println('\n');
#endif
            hitCounter++;

            //target respond timeout
          } else if (sensorData.data[0] == 2) {
#ifdef DEBUG
            Serial.println("===========================================================");
            Serial.println("Target report no hit");
            Serial.println("===========================================================");
            Serial.println('\n');

#endif
            // no correct message
            changeGPIOstatus(ERR);
          }

          //connect to the next target
          state = 9;
        }

        //timeout timer for no response
        else if ((millis() - sendTime) > currentTime + 100) {
#ifdef DEBUG
          Serial.println("===========================================================");
          Serial.println("Target communication timeout");
          Serial.println("No answer from the Target");
          Serial.println("===========================================================");
          Serial.println('\n');
#endif

          changeGPIOstatus(ERR);

          //connect to the next target
          state = 9;

        }
        Serial.flush();

        if (millis() - startTime > gameLength) {
#ifdef DEBUG
          Serial.println("===========================================================");
          Serial.println("Game over");
          Serial.print(hitCounter);
          Serial.println(" hits" );
          Serial.println("===========================================================");
          Serial.println('\n');
#endif
          // visual game over
          rainbowEnd();

          if (hitCounter == 0) {
            // no hits reproted
            changeGPIOstatus(ERR);
            delay(8000);
          } else
          {
            // report hit counter by led blinking
            // blink minimal 10 times
            for (i = 0; i <= hitCounter; i++) {
              changeGPIOstatus(RECV);
              delay(200);
              changeGPIOstatus(OUT);
              delay(800);
            }

            changeGPIOstatus(MENU);
            // in 10 sec starts new game
            delay(10000);
#ifdef DEBUG
            Serial.println("===========================================================");
            Serial.println("Restart the Game :D");
            Serial.println("===========================================================");
            Serial.println('\n');
#endif
          }

          // end of this round
          state = 0;
        }
        break;

      //------------------------------------------------------------------------------------------------------------------------

      // connect to the next target
      case 9:
        currentTime = random(500, 2000);
        // between 0.5 and 2 sec. before next target gets selected
        delay(currentTime);
        state = 7;
        break;
    }
  }
#endif //end -  Server loop



  //___target_loop______________________________________________________________________________________________________________



#ifdef TARGET

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

    //refresh last request timer
    lastRequestTime = millis();

    // reset the message notification
    haveReading = false;
    // change for start

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
        bs[1] = initVal >> 8;
        bs[2] = initVal & 0xFF;
        changeGPIOstatus(ERR);
        initSuccess = false;
      } 
      else {
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
  
  // if the target gets in the last 5 min no request
  if ((millis() - lastRequestTime) > 300000) {
    // not used
#ifdef DEBUG
    Serial.println("===========================================================");
    Serial.println("no request in the last 5 minutes");
    Serial.println("===========================================================");
#endif
    changeGPIOstatus(ERR);
    
    //sensor values are to old
    initSuccess = false;
    
    // refresh last change
    lastRequestTime = millis();
  }

#endif //end -  Target loop
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

#ifdef TARGET
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
#endif

  // triggers everytime something is received by a device
  esp_now_register_recv_cb([](uint8_t *mac, uint8_t *data, uint8_t len) {
    // copy the data of the transfer array to data
    memcpy(&sensorData, data, sizeof(sensorData));

    // Server handling
#ifdef GAMESERVER

    if (targetAction == 1) {
      if ((mac[0] == potentialTargetsMacs[currentTarget][0])
          && (mac[1] == potentialTargetsMacs[currentTarget][1])
          && (mac[2] == potentialTargetsMacs[currentTarget][2])
          && (mac[3] == potentialTargetsMacs[currentTarget][3])
          && (mac[4] == potentialTargetsMacs[currentTarget][4])
          && (mac[5] == potentialTargetsMacs[currentTarget][5])) {
        haveReading = true;
      }
    } else {
      // check if the message comes from the right device
      if ((mac[0] == targetMacs[currentTarget][0])
          && (mac[1] == targetMacs[currentTarget][1])
          && (mac[2] == targetMacs[currentTarget][2])
          && (mac[3] == targetMacs[currentTarget][3])
          && (mac[4] == targetMacs[currentTarget][4])
          && (mac[5] == targetMacs[currentTarget][5])) {
        haveReading = true;
      }
    }
#endif

#ifdef TARGET
    haveReading = true;
#endif
  });
}


//___scan4Targets_______________________________________________________________________________________________________________

#ifdef GAMESERVER
void scanForTargets() {
  int8_t scanResults = WiFi.scanNetworks();
#ifdef DEBUG
  Serial.println("===========================================================");
  Serial.println("Scan for targets");
  Serial.println("===========================================================");
  Serial.println('\n');
#endif

  if (scanResults == 0) {
#ifdef DEBUG
    Serial.println("===========================================================");
    Serial.println("No WiFi devices in AP Mode found");
    Serial.println("===========================================================");
    Serial.println('\n');
#endif

    changeGPIOstatus(ERR);
  }

  else {
#ifdef DEBUG
    Serial.println("===========================================================");
    Serial.print("Found ");
    Serial.print(scanResults);
    Serial.println(" potential Wifi devices ");
    Serial.println("===========================================================");
    Serial.println('\n');
#endif

    for (int i = 0; i < scanResults; ++i) {



      String SSID = WiFi.SSID(i);
      int32_t RSSI = WiFi.RSSI(i);
      String BSSIDstr = WiFi.BSSIDstr(i);

      // Save SSID, RSSI and BSSID for each device found that begins with "LG_"
      if (SSID.indexOf("LG_") == 0) {

        int mac[6];

        if ( 6 == sscanf(BSSIDstr.c_str(), "%x:%x:%x:%x:%x:%x%c", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5] ) ) {
          for (int ii = 0; ii < 6; ++ii ) {
            potentialTargetsMacs[potentialTargets][ii] = (uint8_t) mac[ii];
          }
        }

#ifdef DEBUG
        Serial.println("===============================================");
        Serial.print("Target :");
        Serial.print(i);
        Serial.print(" : ");
        Serial.println(potentialTargets);
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

        // increase the target counter
        potentialTargets++;
      }
    }
  }
  // clean up ram
  WiFi.scanDelete();
  targetAction = 1;
#ifdef DEBUG
  Serial.println("===============================================");
  Serial.println("End of scan methode");
  Serial.print("Potential Targets : ");
  Serial.println(potentialTargets);
  Serial.println("===============================================");
#endif
}
#endif

//___initSensor_________________________________________________________________________________________________________________

#ifdef TARGET
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
#endif

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

//____Game_over_________________________________________________________________________________________________________________

#ifdef GAMESERVER
void rainbowEnd() {
  LED.white();
  delay(1000);
  LED.violette();
  delay(1000);
  LED.red();
  delay(1000);
  LED.yellow();
  delay(1000);
  LED.green();
  delay(1000);
  LED.cyan();
  delay(1000);
  LED.blue();
  delay(1000);
}
#endif

//____Countdown_________________________________________________________________________________________________________________

#ifdef GAMESERVER
void countDown() {
#ifdef DEBUG
  Serial.println("===========================================================");
  Serial.println("Ready");
#endif
  LED.red();
  delay(1500);
#ifdef DEBUG
  Serial.println("SET");
#endif
  LED.yellow();
  delay(1500);
#ifdef DEBUG
  Serial.println("GOOOOOO");
  Serial.println("===========================================================");
  Serial.println('\n');
#endif
  LED.green();
  delay(400);
}
#endif

//______________________________________________________________________________________________________________________________
