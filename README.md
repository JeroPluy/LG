# LG
#### Short Summary:
A game based on [ESP8266 – Witty Cloud Moduls](https://www.ebay.de/itm/ESP8266-Serial-WIFI-Witty-Cloud-Development-Board-ESP-12F-Module-MINI-nodemcu/173615398063?_trkparms=aid%3D1110001%26algo%3DSPLICE.SIM%26ao%3D2%26asc%3D20160323102634%26meid%3D3aa4e928f3964fcbb2f8c6cbc12c2c0f%26pid%3D100623%26rk%3D2%26rkt%3D6%26sd%3D222081069541%26itm%3D173615398063%26pmt%3D0%26noa%3D1%26pg%3D2047675&_trksid=p2047675.c100623.m-1), ESP-Now and a laser pointer. The goal of the game is to hit the blinking ESPs with the laser gun and reach a high score of hits.
It's like lasertag on fixed targets with a real laser.


## Game Instructions
#### Installing Instructions

1. Download the [Arduino IDE](https://www.arduino.cc/en/main/software) and install it. ([Install Guide](https://www.arduino.cc/en/Guide/HomePage))

2. Add the needed libraries. 

   - ESP8266 ([How to add libraries](https://randomnerdtutorials.com/how-to-install-esp8266-board-arduino-ide/))
   - Color   &nbsp; &nbsp; &nbsp;([How to add .zip - libraries](https://www.arduino.cc/en/Guide/Libraries))
   
3. Connect the ESP8266 that will be the server host.

   - Make sure your tool settings are correct
   
   -![Settings in Tools](https://github.com/JeroPlay/LG/blob/master/SecretFiles/Settings%20for%20Tools.png)

4. Open the [GetServerMac]()-program in the IDE

5. Upload the program to the ESP. (`Ctrl + U`)

6. Copy the AP MAC and the STA MAC from the console to the mac.txt file.

  ```
  Console Output Example:
 ===========================================================
 Gameserver
 -----------------------------------------------------------
 This node AP mac:  AF:FA:BC:02:E6:CD
 This node STA mac: AC:FA:BC:02:E6:CD
 ===========================================================
  ```

  7. Change server mac addresses for the targets
      ```   
         // server esp mac addresses for the targets
         #ifdef TARGET

         uint8_t GAMESERVER_ap_mac[]   = {0xEE, 0xFA, 0xBC, 0x0C, 0xE6, 0xAF};
         uint8_t GAMESERVER_sta_mac[]  = {0xEC, 0xFA, 0xBC, 0x0C, 0xE6, 0xAF};

         // init sensor val
         int initVal;

         #endif
      ```
  7. 1. **Open CompleteCodeDV**  
     
  - Comment out TARGET
   
```
// unwanted modes should be commented out
#define DEBUG
//#define TARGET
#define GAMESERVER
```    
7. 2. **Open GamerserverDV**

5. Upload the program the first Time to get. (`Ctrl + U`)

6. Open the 

`(Code row : 68)`
   
```
// server esp mac addresses for the targets
#ifdef TARGET

uint8_t GAMESERVER_ap_mac[]   = {0xEE, 0xFA, 0xBC, 0x0C, 0xE6, 0xAF};
uint8_t GAMESERVER_sta_mac[]  = {0xEC, 0xFA, 0xBC, 0x0C, 0xE6, 0xAF};
```    

4. 2. **Open program for this ESP (Gamerserver / Target)**

6. Installation for this ESP is ready. 
(_You have to do this for every target and for one server_)


## Side Notes
* If you want some printouts on the console, use a \*DV (_DebugVersion_)
* The maimum number of targets per game is now limited to 40 but I haven't tried more than 14 (but it should work with more too) 


