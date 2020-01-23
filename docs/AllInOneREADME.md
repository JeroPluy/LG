#### Installing Instructions

1. Open CompleteCodeDV with the Arduino IDE

2. Change the server Mac addresses for the targets (**don't** copy paste complete mac.txt) `(Code row : 71 - 72)`

    ```
    // server esp mac addresses for the targets
    #ifdef TARGET

    uint8_t GAMESERVER_ap_mac[]   = {0xEE, 0xFA, 0xBC, 0x0C, 0xE6, 0xAF}; 
    uint8_t GAMESERVER_sta_mac[]  = {0xEC, 0xFA, 0xBC, 0x0C, 0xE6, 0xAF};

    // init sensor val
    int initVal;

    #endif
    ```
        
3. Comment out TARGET (`Row: 27`)
   
      ```
      // unwanted modes should be commented out
      #define DEBUG
      //#define TARGET
      #define GAMESERVER
      ``` 
      

3. Upload the program to the *server* ESP. (`Ctrl + U`)

4. Connect the ESP8266 that will be one of the targets.

5. Comment out GAMESERVER and delete the `//` before the `#define TARGET`. (`Row: 28`)

      ```
      // unwanted modes should be commented out
      #define DEBUG
      #define TARGET
      //#define GAMESERVER
      ``` 
6. Upload the program to the *target* ESP. (`Ctrl + U`)

7. Connect the other targets to your PC one after the other and upload the program. No code adjustments are required.

#### Installation complete

### Game process 
see [README.md](README.md)

