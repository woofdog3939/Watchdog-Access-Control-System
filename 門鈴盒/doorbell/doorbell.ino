//SOCKET client端  4/10
#include <string.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h>
#include <Hash.h>
ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;
#define PIN D4
#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel strip = Adafruit_NeoPixel(3, PIN, NEO_GRB + NEO_KHZ800);
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#define RST_PIN      D3        // 讀卡機的重置腳位
#define SS_PIN       D8        // 晶片選擇腳位
LiquidCrystal_I2C lcd(0x27, 16, 2);
#define       LED0      2
#define       BUTTON    D0
int buzzerPin=10;
int           ButtonState;
unsigned long Nowtime = 0;
unsigned long LCDNowtime = 0;
const char* ssid = "Wifi ssid";
const char* password = "WiFi password";
String listen = "";
String DoorST = "close";
bool isread = false; //註冊模式讀卡了嗎?

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
//==================RFID==========================
struct RFIDTag {    // 定義結構
   byte uid[4];
   char *name;
};
struct RFIDTag tags[] = {  // 初始化結構資料
   {{60,209,110,133}, "Arduino"},
   {{0x11,0xA7,0x5D,0x59}, "磁扣"},
   {{0x2B,0x8F,0x27,0x1F}, "磁卡"},
   {{0x93,0xA7,0x5C,0xD7}, "遊遊卡"},
   {{0x1,0x2,0x3,0x4}, "XZ"},
};
byte totalTags = sizeof(tags) / sizeof(RFIDTag);
MFRC522 mfrc522(SS_PIN, RST_PIN);  // 建立MFRC522物件
//==============================================

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");
  pinMode(LED0, OUTPUT);
  pinMode(BUTTON, INPUT);
  pinMode(buzzerPin, OUTPUT);
  strip.begin();
  strip.show();
  lcd.begin(16,2);
  lcd.init();
  SPI.begin();
  mfrc522.PCD_Init();
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  // server address, port and URL
  webSocket.begin("192.168.43.94", 81, "/");
  // event handler
  webSocket.onEvent(webSocketEvent);
  // use HTTP Basic Authorization this is optional remove if not needed
  webSocket.setAuthorization("user", "Password");
  // try ever 5000 again if connection has failed
  webSocket.setReconnectInterval(5000);
}

void loop() {
  webSocket.loop();
  readRFID();
  ButtonState = digitalRead(BUTTON);

  if(ButtonState == LOW){
      Serial.println("叮咚");
      webSocket.sendTXT("button");
      Nowtime = millis();
      lcd.backlight();
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("Please wait");
      delay(500);
  }

  if(listen =="open"){
      noTone(buzzerPin);
      delay(300);
      listen ="";
      DoorST ="open";
      strip.setPixelColor(0, 0, 0, 0);
      strip.setPixelColor(1, 0, 255, 0);
      strip.show();
      tone(buzzerPin, 1000, 500);
      lcd.backlight();
      lcd.clear();
      lcd.setCursor(4, 0);
      lcd.print("Welcome");
      lcd.setCursor(1, 1);
      lcd.print("Door Unlocked");
      Serial.println("開門");
      Nowtime = millis();
  }else if(listen =="close"){
      noTone(buzzerPin);
      delay(300);
      listen ="";
      DoorST ="close";
      strip.setPixelColor(0, 255, 0, 0);
      strip.setPixelColor(1, 0, 0, 0);
      strip.show();
      tone(buzzerPin, 1000, 100);
      delay(120);
      tone(buzzerPin, 1000, 100);
      Serial.println("關門");
      //webSocket.sendTXT("close");    
      lcd.backlight();
      lcd.clear();
      lcd.setCursor(2, 1);
      lcd.print("Door locked");
      Nowtime = millis();
  }else if(listen.startsWith("send")){
      Serial.println(listen.substring(5));
      Nowtime = millis();
      lcd.backlight();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Message:");
      lcd.setCursor(0, 1);
      lcd.print(listen.substring(5));
      Nowtime = millis();
      listen ="";
  }else if(listen =="noB"){
      noTone(buzzerPin);
      listen ="";
  }else if(listen =="error"){
      noTone(buzzerPin);
      delay(300);
      tone(buzzerPin, 300, 100);
      delay(150);
      tone(buzzerPin, 300, 100);
      delay(200);
      lcd.backlight();
      lcd.clear();
      lcd.setCursor(3, 0);
      lcd.print(" NO Access");
      lcd.setCursor(2, 1);
      lcd.print("Door locked");
      Nowtime = millis();
      listen ="";
  }else if(listen =="signupMode"){
       listen ="";
      isread = false;
      lcd.backlight();
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("Sign Up Mode");
      lcd.setCursor(0, 1);
      lcd.print("Please read card");
      while(1){
        webSocket.loop();
        if(isread){
          break;
        }else if(listen =="signupfinish"){
          break;
        }
        readRFID();
        listen ="";
        delay(100);
      }
      tone(buzzerPin, 1000, 100);
      delay(120);
      tone(buzzerPin, 1000, 100);
      delay(120);
      tone(buzzerPin, 1000, 100);
      delay(120);
      tone(buzzerPin, 1000, 100);
      lcd.noBacklight();
      lcd.clear();
  }else{
      listen ="";
  }


    if(millis()-Nowtime > 9000) {   //過5秒關閉LCD     
      lcd.noBacklight();
      lcd.clear();
   }
/*    if(millis()-LCDNowtime > 9000) {   //過5秒關閉LCD     
      lcd.noBacklight();
      lcd.clear();
   }*/
   
}
//==================讀取RFID==========================
void readRFID(){
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {  // 確認是否有新卡片
      tone(buzzerPin, 860);
      byte *id = mfrc522.uid.uidByte;   // 取得卡片的UID
      byte idSize = mfrc522.uid.size;   // 取得UID的長度
      String rfidID="";
      for (byte i = 0; i < idSize; i++) {  // 逐一顯示UID碼
        rfidID+=(id[i]);
      }
      Serial.println("");
      Serial.println(rfidID);
      isread =true;
      webSocket.sendTXT("rfid"+rfidID);
      lcd.backlight();
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("Loading...");
      Nowtime = millis();
      mfrc522.PICC_HaltA();  // 讓卡片進入停止模式      
    }
}
//===============================================
