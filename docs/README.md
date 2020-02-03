# LaserGame
#### Short Summary:
A game based on [ESP8266 – Witty Cloud Moduls](https://www.ebay.de/itm/ESP8266-Serial-WIFI-Witty-Cloud-Development-Board-ESP-12F-Module-MINI-nodemcu/173615398063?_trkparms=aid%3D1110001%26algo%3DSPLICE.SIM%26ao%3D2%26asc%3D20160323102634%26meid%3D3aa4e928f3964fcbb2f8c6cbc12c2c0f%26pid%3D100623%26rk%3D2%26rkt%3D6%26sd%3D222081069541%26itm%3D173615398063%26pmt%3D0%26noa%3D1%26pg%3D2047675&_trksid=p2047675.c100623.m-1), ESP-Now and a laser pointer. The goal of the game is to hit the blinking ESPs with the laser gun and reach a high score of hits.
It's like Laser Tag on fixed targets with a real laser.

If you ever want to shoot with a laser pointer at an actual target, this game is a cheap version to try this out and learn more about Mc's and ESP-Now.



### Table of Contents

   1. [Installing Instructions](README.md#installing-instructions)
   2. [Game Instructions](README.md#game-instructions)
   3. [The project history](README.md#the-project-history)
   4. [The 2 versions](README.md#the-2-versions)
   5. [About the creater](README.md#about-the-creater)

### Installing Instructions

1. Download the [Arduino IDE](https://www.arduino.cc/en/main/software) and install it. ([Install Guide](https://www.arduino.cc/en/Guide/HomePage))

2. Add the needed libraries. (`LG/Installation/..`) 

   - ESP8266 ([How to add libraries](https://randomnerdtutorials.com/how-to-install-esp8266-board-arduino-ide/))
   - Color   &nbsp; &nbsp; &nbsp;([How to add .zip - libraries](https://www.arduino.cc/en/Guide/Libraries))  (The library is in the installation directory)
   
3. Connect the ESP8266 that will be the server host to your PC.

   - Make sure your tool settings are correct (Port could be different)
   
   ![Settings in Tools](../SecretFiles/Settings%20for%20Tools.png)

4. Open the [GetServerMac](../Installation/GetServerMac)-program in the IDE

5. Upload the program to the ESP. (`Ctrl + U`)

6. Open the serial monitor. (`Ctrl + Shift + M` or `Tools -> Serial Monitor` )

7. Copy the AP MAC and the STA MAC from the console to the [Server-Mac.txt](../Installation/Server-Mac.txt) file.

   ```
   Console Output Example:
   ===========================================================
   Gameserver
   -----------------------------------------------------------
   This node AP mac:  AF:FA:BC:02:E6:CD
   This node STA mac: AC:FA:BC:02:E6:CD
   ===========================================================
   ```
   
  8. Open [LGGameserver](../Game/LGGameserver/LGGameserver.ino) with the Arduino IDE.
  
  9. Upload the program to the ESP. (`Ctrl + U`)
  
 10. Open [LGTarget](../Game/LGTarget/LGTarget.ino) with the Arduino IDE.
 
 11. Change the server Mac addresses for the targets (**don't** copy paste complete mac.txt) `(Code row : 67 - 68)`
 ```
  
       // server esp mac addresses for the targets
       #ifdef TARGET

       uint8_t GAMESERVER_ap_mac[]   = {0xEE, 0xFA, 0xBC, 0x0C, 0xE6, 0xAF}; 
       uint8_t GAMESERVER_sta_mac[]  = {0xEC, 0xFA, 0xBC, 0x0C, 0xE6, 0xAF};

       // init sensor val
       int initVal;

       #endif
```
    
 12. Connect an ESP8266 that will be one of the targets.
 
 13. Upload the program to the ESP. (`Ctrl + U`)
 
 14. Repeat the steps 12 and 13 for every target.
 
 15. Installation completed.

 ### Game instructions
 
The server starts the program with the initialization and the connections to the targets. Then the targets initialize their LDR sensor. 
If the sensor sends a valid value, the LED of the target goes out, otherwise the LED lights up red and the target cannot be used.
The game starts with the first hit of the target with **green LED**. The server counts down and the game begins.
A random target lights up and the target is waiting to be hit by the laser. (Remember that the sensor is not the LED)
If the target has been hit, the server LED lights up green, otherwise red. The goal of the game is to hit most of the targets in a given time. After the time has expired, the LED flashes for each hit. 
Just wait for a single green target to play again and start again.

([Sequence diagram](LG%20SD.pdf) for visual game instructions)
 
 ### The project history
 
 The idea for this game was developed at the *Hackathon 2018* by a small group of [telematics students from TH Wildau](https://en.th-wildau.de/index.php?id=23510). First of all, it should be implemented with a Raspberry Pi 3.0 as server, several ESP8266, Wifi as connection protocol and a normal monitor as output device. Due to some ["problems"](https://www.meinehochschulebehindertdaswlan.de/en/) this idea did not work well and the technical side of the project had to be revised.
 
At this point, I joined the project and build the code and project you know from this repository.

The first adjustments involved changing the communication protocol from Wifi to ESPNow, which worked quite well, and changing the server from Raspberry-Pi to another ESP8266 because the Pi cannot handle EPSNow.


This version is still a very basic one, and so it is also very expandable. I want to provide this as the basis for further adjustments and as a running version of itself.
 
 ### The 2 versions
 
 The project includes two versions of code. The first one is the complete code in on .ino file so you have everything concentrated. 
 For reverse-engineer, this version is pretty confusing and it is just for a fast and easy import and installation.
 The second code is separated into two programs - the server program and the target program. It is easier to understand and to reverse-engineer.
 It is up to you which version you want to use. Both have the same program sequences.
 
 VERSION 1 : [LG_AllInOne](../Game/LG_AllInOne)
 
 VERSION 2 : [LG_Server_Target](../Game/LG_Server_Target)

 ### About the creater
      

## Side Notes
* If you want some printouts on the console, use the \*DV (_DebugVersion_)
* The maximum number of targets per game is now limited to 40 but I haven't tried more than 14 (but it should work with more too) 


