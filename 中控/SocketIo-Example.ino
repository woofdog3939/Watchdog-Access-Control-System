//SOCKET SERVER端10/04

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include <FirebaseArduino.h>
#include <Servo.h>
#define firebaseURl "xxxxxxxxxxxxxxxxxxxxx"                 //Firebase設定
#define authCode "xxxxxxxxxxxxxxxxxxxxxxxx"
#include <DS1302.h>
#include <ArduinoJson.h>
const char* ssid = "WiFi ssid";
const char* password = "WiFi password";
String listen = "";
unsigned long Nowtime = 0;
unsigned long autoCloseTime = 0;
Servo myservo;
int wakeFlag=1;  //卡喚醒
bool warringstate; //卡警報
int doorlockFlag=1;//卡重複傳open
int doortimeFlag=1;//卡開關門時間
int doorclosetimeFlag=1;//卡自動關門
int pirTime=0;
String readrfid = "";
int pir=1;
int led=10;
DS1302 rtc(D5, D6, D7);//時鐘
String td = "";
String yyyy = "";
String mm = "";
String dd = "";
String t = "";
String t1 = "";
String doortime = "";
bool lock;


WebSocketsServer webSocket = WebSocketsServer(81);
ESP8266WebServer server(80);

void setupFirebase() {
    Firebase.begin(firebaseURl, authCode);
}
void setup() {
  rtc.halt(false);
  //rtc.writeProtect(false);
  Serial.begin(115200);
  /*
  rtc.setDOW(TUESDAY);      //設定禮拜幾
  rtc.setTime(14, 34, 0);     // 設定時間
  rtc.setDate(24, 4, 2018);   // 設定日期
  */
  WiFi.begin(ssid, password);
  Serial.println("");
  pinMode(D1, INPUT);//磁簧開關
  myservo.attach(D0); //馬達
  pinMode(D2,INPUT);//人體紅外線
  pinMode(led,OUTPUT);//門口燈

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    
    server.on("/", [](){
    server.send(200, "text/html", WebPage);
    });
    server.begin();
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    setupFirebase();
}

void loop() { 
  webSocket.loop();
  readRFID();
  getclock();
  wakeUp();
  pirled();
  lock = Firebase.getBool("status/lockST");
  bool warning = Firebase.getBool("status/warning");//warning
  String LCD=Firebase.getString("APP/LCD");
  bool signupMode = Firebase.getBool("signupRFID/signupMode");
  String newRFIDuser=Firebase.getString("signupRFID/newRFIDuser");

 if(signupMode==true){
   Serial.println("註冊模式");
   
   webSocket.broadcastTXT("signupMode");
   String newrfid;
   bool ifRead =false;
   while (ifRead != true){ 
    webSocket.loop();                  //String testdata = Firebase.getString("testdata");
    signupMode = Firebase.getBool("signupRFID/signupMode");
    if(listen.startsWith("rfid")){      //if(testdata!="")
      ifRead =true;
      Serial.println(ifRead);
      Serial.println("資料來了");
      Serial.println("收到");
      newrfid=listen.substring(4);
      Serial.println(newrfid);
      Serial.println(newRFIDuser);
      Firebase.setString("account/"+newRFIDuser+"/RFID",newrfid);
      Firebase.setString("rfid/"+newRFIDuser+"/rfid",newrfid);
      newrfid="";
      newRFIDuser="";
      
    }else if(signupMode==false){
     break;
     
    }
    delay(500);    
    Serial.println(signupMode);
    Serial.print(".");
   }
   Firebase.setString("signupRFID/newRFIDuser","");
   Firebase.setBool("signupRFID/signupMode",false);
   webSocket.broadcastTXT("signupfinish");
   Serial.println("註冊完成");
   listen="";
  }
   if(digitalRead(D1)==1){//門蓋著      
      Firebase.setBool("status/doorST",false);
        if(lock ==1 && doorlockFlag==1){
          myservo.write(180);
          webSocket.broadcastTXT("open");
          doorlockFlag=0;
        }else if(lock ==0 && doorlockFlag==0){
          myservo.write(0);
          webSocket.broadcastTXT("close");
          doorlockFlag=1;
        }
   }else{//門開著
      Firebase.setBool("status/doorST",true);
   }
       if(digitalRead(D1)==0&&doortimeFlag==1){
        doortimeFlag=0;
        Firebase.pushString("openTime/"+t1,t);
    }
    if(digitalRead(D1)==1&&doortimeFlag==0){
        doortimeFlag=1;
        Firebase.pushString("closeTime/"+t1,t);
    }
     if(LCD!=""){
       webSocket.broadcastTXT("send "+ LCD);
       Serial.println("傳LCD的字"); 
       Firebase.setString("APP/LCD","");
    }
  if(digitalRead(D1)==0&&lock==0){
      Firebase.setBool("status/warning",true);
  }
  if(warning==1&&warringstate==1){
     webSocket.broadcastTXT("war");//war
     Serial.println("開警報");
     warringstate=0;
  }else if(warning==0&&warringstate==0){
     webSocket.broadcastTXT("stop");//stop
     Serial.println("關警報");
     warringstate=1;
  }
   if(lock==1&&digitalRead(D1)==1&&doorclosetimeFlag==1){
       autoCloseTime = millis();
       delay(100);
       doorclosetimeFlag=0;
       Serial.println("開始算");
    }
    if(digitalRead(D1)==0&&doorclosetimeFlag==0){
        doorclosetimeFlag=1;
    }
    if(lock==0){
        doorclosetimeFlag=1;
    }
    if(millis()-autoCloseTime>10000 && doorclosetimeFlag==0){
        Firebase.setBool("status/lockST",false);
        Serial.println("鎖門了");
        doorclosetimeFlag=1;
    }

}
void(* resetFunc) (void) = 0;//重製  <--
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length){
   if (type == WStype_TEXT){
    for(int i = 0; i < length; i++){
      //Serial.print((char) payload[i]);
      listen += ((char) payload[i]);
   }
    Serial.println("listen: "+ listen);
    Serial.println();
   }
}
void readRFID(){
    if(listen.startsWith("rfid")){     
      Serial.println("收到");
      readrfid=listen.substring(4);
      Serial.println(readrfid);
      int arraycount = 0;//抓長度
      const ArduinoJson::JsonObject& growSchedule = Firebase.get("rfid").getJsonVariant().asObject();
      for (ArduinoJson::Internals::ListConstIterator<JsonPair> jpi = growSchedule.begin(); jpi != growSchedule.end(); ++jpi) {
        arraycount++;
      }
      if(arraycount==0){         //<--
           Serial.println("重置");
           resetFunc();//重置
      }
      String arrayRFID[arraycount];
      arraycount=0;
      //抓account資料
      for (ArduinoJson::Internals::ListConstIterator<JsonPair> jpi = growSchedule.begin(); jpi != growSchedule.end(); ++jpi) {
        JsonPair jp = *jpi;
        JsonObject& o = jp.value.asObject();
        String n = o.get<String>("rfid"); // < what is name for?
        String key = String(jp.key); 
        arrayRFID[arraycount]=n;
        arraycount++;
      }
      //比對資料
      int yesCard=0;
      for(int check=0;check<arraycount;check++){
        if(arrayRFID[check]==readrfid){
          Serial.println("找到了:"+arrayRFID[check]);
          yesCard++;
          break;
        }else{
          Serial.println("找不到");
        }
      }
      if(yesCard==1){
        if(lock){
            Firebase.setBool("status/lockST",false);
            webSocket.broadcastTXT("noB");
        }else{
            Firebase.setBool("status/lockST",true);
            webSocket.broadcastTXT("noB");
        }
      }else{
         Firebase.setBool("status/lockST",false);
         webSocket.broadcastTXT("error");//error
         Serial.println("傳畢錯卡");
      }
      readrfid="";
      listen="";
    }else if(listen=="button"){
      webSocket.broadcastTXT("button");//button
      Firebase.setBool("status/butten",true);
      Firebase.pushString("buttonTime/"+t1,t);//按門鈴時間
      Serial.println("傳叮咚");
      delay(200); 
      Firebase.setBool("status/butten",false);
      listen="";
    }else{
      listen="";
    }
}
void wakeUp(){    //喚醒
    if(wakeFlag==1) {   
     Nowtime = millis();
     delay(300);
     wakeFlag=0;
    }
    if(millis()-Nowtime>10000){
      webSocket.broadcastTXT("wake");
      Serial.println("喚醒");
      wakeFlag=1;
    }
}
void getclock(){
  td=rtc.getDateStr();
  yyyy=td.substring(6,10);
  mm=td.substring(3,5);
  dd=td.substring(0,2);
  t=yyyy+"-"+mm+"-"+dd+" "+rtc.getTimeStr();
  t1=yyyy+"-"+mm+"-"+dd;
}
void pirled(){
    if(digitalRead(D2)==HIGH&&pir==1){
      digitalWrite(led,HIGH);
      pirTime = millis();
      delay(100);
      pir=0;
      Serial.println("有人");
    }
    else if(millis()-pirTime>5000&&pir==0){
      digitalWrite(led,LOW);
      Serial.println("關燈");
      pir=1;
    }  
}
