#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h>
#include <Hash.h>
ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;
#define       SKEAKER      D7
const char* ssid = "WiFi ssid";
const char* password = "WiFi password";
unsigned long Nowtime = 0;
String listen = "";
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
   if (type == WStype_TEXT){
    for(int i = 0; i < length; i++) {
      //Serial.print((char) payload[i]);
      listen += ((char) payload[i]);
    }
    Serial.println("listen: "+ listen);
    Serial.println();
   }
}
void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");
  pinMode(SKEAKER, OUTPUT);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  webSocket.begin("192.168.43.94", 81, "/");
  webSocket.onEvent(webSocketEvent);
  webSocket.setAuthorization("user", "Password");
  webSocket.setReconnectInterval(5000);
}
void loop() {
  webSocket.loop();
  //webSocket.sendTXT("chby");
  //delay(1000);
  if(listen =="button"){
      listen ="";
      tone(SKEAKER,1200);
      delay(500);
      tone(SKEAKER,932);
      delay(1600);
      noTone(SKEAKER);
  }
  if(listen =="war"){
      listen ="";
      while(1){
        for (int i=150; i<1800; i+=15) {
          tone(SKEAKER, i,10); 
          delay(2);
        }
        for (int i=1800; i>150; i-=15) { 
          tone(SKEAKER,i,10);
          delay(2);
        }
        webSocket.loop(); 
        if(listen =="stop"){
          noTone(SKEAKER);
          break;
        }else{
          listen ="";
        }
      }
  }else{
      listen ="";
  }
  
}
