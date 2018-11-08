#include <string.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h>
#include <Hash.h>
ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;
#include <Adafruit_NeoPixel.h>
uint16_t i, j;
#define ADD true  //呼吸燈邏輯
#define SUB false //呼吸燈邏輯
int val = 0;
int whileA=0;
boolean stat = ADD;
#define       LED0      2         // WIFI Module LED
#define PIN D1
Adafruit_NeoPixel strip = Adafruit_NeoPixel(102, PIN, NEO_GRB + NEO_KHZ800);
unsigned long Nowtime = 0;
const char* ssid = "WiFi ssid";
const char* password = "Wifi password";
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
  Serial.begin(115200);           // Computer Communication
  WiFi.begin(ssid, password);
  Serial.println("");

  strip.begin();
  strip.show();
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

//====================================================================================
  
  void loop(){
    webSocket.loop();

    if(listen == "open"){   //開門  綠燈
      listen = "";
      Serial.println("開門");
      //rainbow(20);
      for(i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, 0, 255, 0);
      }
      strip.show();
      delay(2000);
      for(i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, 0, 0, 0);
      }
      strip.show();
     }
    else if(listen == "close"){    //關門
      listen = "";
      Serial.println("關門");
    }
    else if(listen == "wake"){   //喚醒
      listen = "";
    }
    else if(listen == "error"){  //逼錯卡  紅燈閃爍
      listen = "";
        for(i=0; i<strip.numPixels(); i++) {//red
          strip.setPixelColor(i, 255, 0, 0);
        }
        strip.show();
        delay(300);
        for(i=0; i<strip.numPixels(); i++) {//no
          strip.setPixelColor(i, 0, 0, 0);
        }
        strip.show();
        delay(300);
        for(i=0; i<strip.numPixels(); i++) {
          strip.setPixelColor(i, 255, 0, 0);
        }
        strip.show();
        delay(300);
        for(i=0; i<strip.numPixels(); i++) {
          strip.setPixelColor(i, 0, 0, 0);
        }
        strip.show();
        delay(300);
      for(i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, 255, 0, 0);
      }
      strip.show();
      delay(1500);
      for(i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, 0, 0, 0);
      }
      strip.show();
    }
    else if(listen == "war"){   //警報
      listen = "";
      Serial.println("開警報");
        while(1){
            for(j=0;j<5;j++){
              for(i=0; i<52; i++) {
                strip.setPixelColor(i, 2550, 0, 0);
              }
              for(i=52; i<103; i++) {
                  strip.setPixelColor(i, 0, 0, 255);
              }
              strip.show();
              delay(50);
              for(i=0; i<strip.numPixels(); i++) {
                  strip.setPixelColor(i, 0, 0, 0);
              }
              strip.show();
              delay(50);
            }
            webSocket.loop();
            if(listen == "stop"){
              break;
            }else{
              listen = "";
            }
            for(j=0;j<5;j++){
              for(i=0; i<52; i++) {
                strip.setPixelColor(i, 0, 0, 255);
              }
              for(i=52; i<103; i++) {
                  strip.setPixelColor(i, 255, 0, 0);
              }
              strip.show();
              delay(50);
              for(i=0; i<strip.numPixels(); i++) {
                  strip.setPixelColor(i, 0, 0, 0);
              }
              strip.show();
              delay(50);
            }
            webSocket.loop();
            if(listen == "stop"){
              break;
            }else{
              listen = "";
            }
        }
    }else if(listen == "signupMode"){  //註冊模式
        listen = "";
        for(i=0; i<102; i++) {
          strip.setPixelColor(i, 255, 225, 0);
         }
        strip.show();
        delay(1000);
        while(1){
           webSocket.loop();
            if(listen == "signupfinish"){
              break;
            }else{
              listen = "";
            }
            delay(100);
        }
        Serial.println("註冊玩");
        for(i=0; i<strip.numPixels(); i++) {//no
          strip.setPixelColor(i, 0, 0, 0);
        }
        strip.show();
    }else if(listen.startsWith("send")){  //收到訊息
      listen = "";
      /*for(i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, 0, 190, 255);
      }
       strip.show();
       delay(3000);*/
       whileA=0;
        while(whileA!=1200){
         for(i=0; i<strip.numPixels(); i++) {
          uint32_t color = strip.Color(val, val, val);
          strip.setPixelColor(i, color, color, color);
         }
         if(val>=120)
            stat = SUB;
          if(val<=0)
            stat = ADD;
           
          strip.show();
          delay(5);
          if(stat==SUB) val --;
          else if(stat==ADD) val++;
          whileA++;
          //Serial.println(whileA);
       }
    }else{  //RGB
      
      /* for(j=0; j<256; j++) {    
        for(i=0; i<strip.numPixels(); i++) {
          strip.setPixelColor(i, Wheel((i+j) & 255));
        }
        strip.show();
        delay(15);
        webSocket.loop();
        if(listen == "open"||listen == "error"){
           j=256;
        }
      }*/
      for(j=0; j<256*5; j++) {
        for(i=0; i< strip.numPixels(); i++) {
          strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
        }
        listen = "";
        strip.show();
        delay(15);
        webSocket.loop();
        if(listen == "open"||listen == "error"||listen.startsWith("send")||listen == "war"||listen == "close"||listen=="signupMode"){
          j=256*5;
        }
      }
    }
}

//====================================================================================
void rainbow(uint8_t wait) {
  uint16_t i, j;
  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
