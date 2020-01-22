#### Installing Instructions

1. Change server mac addresses for the targets (**not** copy paste complete mac.txt)
       
       ```
           // server esp mac addresses for the targets
          #ifdef TARGET

           uint8_t GAMESERVER_ap_mac[]   = {0xEE, 0xFA, 0xBC, 0x0C, 0xE6, 0xAF}; 
           uint8_t GAMESERVER_sta_mac[]  = {0xEC, 0xFA, 0xBC, 0x0C, 0xE6, 0xAF};

           // init sensor val
           int initVal;

           #endif
        ```
        
2. Open CompleteCodeDV  
  
    - Comment out TARGET
   
      ```
      // unwanted modes should be commented out
      #define DEBUG
      //#define TARGET
      #define GAMESERVER
      ``` 
      

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
