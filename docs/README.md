# LaserGame
#### Short Summary:
A game based on [ESP8266 – Witty Cloud Moduls](https://www.ebay.de/itm/ESP8266-Serial-WIFI-Witty-Cloud-Development-Board-ESP-12F-Module-MINI-nodemcu/173615398063?_trkparms=aid%3D1110001%26algo%3DSPLICE.SIM%26ao%3D2%26asc%3D20160323102634%26meid%3D3aa4e928f3964fcbb2f8c6cbc12c2c0f%26pid%3D100623%26rk%3D2%26rkt%3D6%26sd%3D222081069541%26itm%3D173615398063%26pmt%3D0%26noa%3D1%26pg%3D2047675&_trksid=p2047675.c100623.m-1), ESP-Now and a laser pointer. The goal of the game is to hit the blinking ESPs with the laser gun and reach a high score of hits.
It's like lasertag on fixed targets with a real laser.

If you ever want to shoot with a laser pointer at an actual target, this game is a cheap version to try this out and learn more about Mc's and ESP-Now.


## Game Instructions

### Table of Contents

   1. [Installing Instructions](https://github.com/JeroPlay/LG/blob/master/docs/README.md#installing-instructions)
   2. [The 2 versions](https://github.com/JeroPlay/LG/blob/master/docs/README.md#the-2-versions)
   3. [The project](https://github.com/JeroPlay/LG/blob/master/docs/README.md#the-project)
   4. [about the creater](https://github.com/JeroPlay/LG/blob/master/docs/README.md#about-the-creater)

### Installing Instructions

1. Download the [Arduino IDE](https://www.arduino.cc/en/main/software) and install it. ([Install Guide](https://www.arduino.cc/en/Guide/HomePage))

2. Add the needed libraries. 

   - ESP8266 ([How to add libraries](https://randomnerdtutorials.com/how-to-install-esp8266-board-arduino-ide/))
   - Color   &nbsp; &nbsp; &nbsp;([How to add .zip - libraries](https://www.arduino.cc/en/Guide/Libraries))
   
3. Connect the ESP8266 that will be the server host to your PC.

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
   
  7. You will find the remaining installation instructions in the READMEs of the variants
  
      - [AllInOne]()     
      - [Server-Target]()
      
      
 ### The 2 versions
 
 The project includes two versions of code. The first one is the complete code in on .ino file so you have everything concentrated. 
 For reversengineer this version is pretty confusing and it is just for a fast an esay import and installation.
 The second code is separated in two programs - the server program and the target program. It is easier to understand and to reversengineer.
 It is up to you which version you want to use. Both have the exact same program sequences.
 
 VERSION 1 : [AllInOne]()
 
 VERSION 2 : [Server-Target]()
 
 ### The project
  
 ### about the creater
      

## Side Notes
* If you want some printouts on the console, use a \*DV (_DebugVersion_)
* The maimum number of targets per game is now limited to 40 but I haven't tried more than 14 (but it should work with more too) 

