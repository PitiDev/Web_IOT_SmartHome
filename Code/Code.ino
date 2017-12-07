#include <AuthClient.h>
#include <MicroGear.h>
#include <MQTTClient.h>
#include <SHA1.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <MicroGear.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

#define LEDPIN1 D0
#define LEDPIN2 D1
#define LEDPIN3 D2
#define LEDPIN4 D3

#define EEPROM_STATE_ADDRESS 128

// Please visit netpie.io to get keys
#define APPID   "aaaaa"       // AppID from NetPie e.g. "MySwitch"
#define KEY     "bbbbbbb"      //GearKey from NetPie e.g. "4DPanXKaSdb2VrT"
#define SECRET  "cccccccccccccc"   //GearSecret from NetPie e.g. "ZgrXbHsaVp7TI8xW5oEcAqvY3"
#define ALIAS   "piesonoff"

const char* ssid = "msi";
const char* password = "12345678";
WiFiClient client;
AuthClient *authclient;

unsigned char state = 0;   //state is status of o/p pin keep in binary form

MicroGear microgear(client);

void sendState() {
  Serial.print("state is-> ");Serial.println(state,BIN);
  //------------------------channel 1--------------------------//
  if (state & B00000001)                        // Test bit D0 is HIGH ?
  {
    microgear.publish("/piesonoff/state","ch1-on");
    Serial.print("  Send data out --> ");Serial.println("ch1-on");
    }
  if ((state | B11111110)==B11111110)           // Test bit D0 is LOW ?
   {
    microgear.publish("/piesonoff/state","ch1-off");
    Serial.print("  Send data out --> ");Serial.println("ch1-off");    
    }
  //------------------------channel 2--------------------------//
  if (state & B00000010)                        // Test bit D1 is HIGH ?
  {
    microgear.publish("/piesonoff/state","ch2-on");
    Serial.print("  Send data out --> ");Serial.println("ch2-on");     
    }
  if ((state | B11111101)==B11111101)           // Test bit D1 is LOW ?
  {
    microgear.publish("/piesonoff/state","ch2-off");
    Serial.print("  Send data out --> ");Serial.println("ch2-off");     
    }
  //------------------------channel 3--------------------------//
  if (state & B00000100)                        // Test bit D2 is HIGH ?
  {
    microgear.publish("/piesonoff/state","ch3-on");
    Serial.print("  Send data out --> ");Serial.println("ch3-on");     
    }
  if ((state | B11111011)==B11111011)           // Test bit D2 is LOW ?
  {
    microgear.publish("/piesonoff/state","ch3-off");
    Serial.print("  Send data out --> ");Serial.println("ch3-off");     
    } 
  //------------------------channel 4--------------------------//
  if (state & B00001000)                        // Test bit D3 is HIGH ?
  {
    microgear.publish("/piesonoff/state","ch4-on");
    Serial.print("  Send data out --> ");Serial.println("ch4-on");     
    }
  if ((state | B11110111)==B11110111)           // Test bit D3 is LOW ?
  {
    microgear.publish("/piesonoff/state","ch4-off");
    Serial.print("  Send data out --> ");Serial.println("ch4-off");     
    }               
}
void updateIO() {
  if (state & B00000001)digitalWrite(LEDPIN1, LOW);else digitalWrite(LEDPIN1, HIGH);
  if (state & B00000010)digitalWrite(LEDPIN2, LOW);else digitalWrite(LEDPIN2, HIGH);
  if (state & B00000100)digitalWrite(LEDPIN3, LOW);else digitalWrite(LEDPIN3, HIGH); 
  if (state & B00001000)digitalWrite(LEDPIN4, LOW);else digitalWrite(LEDPIN4, HIGH);   
}

void onMsghandler(char *topic, uint8_t* msg, unsigned int msglen) {
  char m = *(char *)msg;
  Serial.print("Incoming message <-- ");
  msg[msglen] = '\0';
  Serial.println((char *)msg);
    if (m == 'A')    state |=B00000001;   //set bit D0 to HIGH
    else if(m == 'a')state &=B11111110;   //clear bit D0 to LOW    
    else if(m == 'B')state |=B00000010;   //set bit D1 to HIGH    
    else if(m == 'b')state &=B11111101;   //clear bit D1 to LOW
    else if(m == 'C')state |=B00000100;   //set bit D2 to HIGH    
    else if(m == 'c')state &=B11111011;   //clear bit D2 to LOW
    else if(m == 'D')state |=B00001000;   //set bit D3 to HIGH    
    else if(m == 'd')state &=B11110111;   //clear bit D3 to LOW    
   
    EEPROM.write(EEPROM_STATE_ADDRESS, state);
    EEPROM.commit();
    updateIO(); 
    sendState();
  }

void onConnected(char *attribute, uint8_t* msg, unsigned int msglen) {
  Serial.println("Connected to NETPIE...");
  sendState();
}

void setup() {
    Serial.begin(115200);
    Serial.println("Starting...");

    pinMode(LEDPIN1, OUTPUT);
    pinMode(LEDPIN2, OUTPUT);
    pinMode(LEDPIN3, OUTPUT);
    pinMode(LEDPIN4, OUTPUT);
    
    EEPROM.begin(512);
    state = EEPROM.read(EEPROM_STATE_ADDRESS);    
    updateIO();

    WiFiManager wifiManager;
    wifiManager.setTimeout(180);

    if(!wifiManager.autoConnect("PieSonoff")) {
      Serial.println("Failed to connect and hit timeout");
      delay(10);
      ESP.reset();
      delay(10);
    }
  
    microgear.on(MESSAGE,onMsghandler);
    microgear.on(CONNECTED,onConnected);

    microgear.init(KEY,SECRET,ALIAS);
    microgear.connect(APPID);
}

void loop() {
  if (microgear.connected()) {    
    microgear.loop();
  }
  else {
    Serial.println("connection lost, reconnect...");
    microgear.connect(APPID);
  }
}
