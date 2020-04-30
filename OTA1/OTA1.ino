#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <FastLED.h>

#define leftled 16
#define rightled 5
#define peltier 13
#define buzzer 12
#define ring 4
#define square 0
#define ws 14

const char* ssid = "Sagemcom 2.4 GHz";
const char* password = "............";
const char* mqtt_server = "192.168.0.33";

int red = 255;
int green = 255;
int blue = 255;

boolean ron = 0;
boolean son = 0;
boolean won = 0;

String IncomingString="";
boolean StringReady = 0;

WiFiClient espClient;
PubSubClient client(espClient);

CRGB rings[16];
CRGB squares[16];
CRGB wss[58];

void startws(){
  for(int i = 0; i < 16; i++) 
    rings[i] = CHSV(0, 0, 0);
  FastLED.show(); 

  for(int i = 0; i < 16; i++) 
    squares[i] = CHSV(0, 0, 0);
  FastLED.show(); 

  for(int i = 0; i < 58; i++) 
    wss[i] = CHSV(0, 0, 0);
  FastLED.show(); 
}

void setup() 
{
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(5000);
    ESP.restart();
  }

  FastLED.addLeds<NEOPIXEL, ring>(rings, 16);
  FastLED.addLeds<NEOPIXEL, square>(squares, 16);
  FastLED.addLeds<NEOPIXEL, ws>(wss, 58);
  LEDS.setBrightness(255);
  startws();
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
  pinMode(buzzer, OUTPUT);
  pinMode(peltier, OUTPUT);
  pinMode(leftled, OUTPUT);
  pinMode(rightled, OUTPUT);

  digitalWrite(peltier, HIGH);
  digitalWrite(leftled, HIGH);
  digitalWrite(rightled, HIGH);
  
  ArduinoOTA.begin();
}

void loop() 
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  while(Serial.available()){
    IncomingString = Serial.readStringUntil(',');
    StringReady = 1;
    delay(1);
  }
  if(StringReady){ 
    if(IncomingString.length() <= 5 && IncomingString.length() >= 3){
        String buff = "";
        for(int i = 1; i < IncomingString.length(); i++){
          if(IncomingString.length() == 3 && i == 1){
             buff = '0';
          }
          if(i == IncomingString.length() - 2){
             buff += '.';
          }
          buff += IncomingString[i];
        }
        char* cstr1 = new char [buff.length()+1];
        strcpy(cstr1, buff.c_str());

        if(IncomingString[0] == '3'){
          client.publish("/home/espsecond/5vrail/state", cstr1);
        }
        else if(IncomingString[0] == '4'){
          client.publish("/home/espsecond/12vrail/state", cstr1);
        }
        else if(IncomingString[0] == '2'){
          client.publish("/home/espsecond/12vamp/state", cstr1);
        }
        else if(IncomingString[0] == '1'){
          client.publish("/home/espsecond/5vamp/state", cstr1);
        }
        delete cstr1;
    }
    IncomingString = "";
    StringReady = 0;
  }
  
  ArduinoOTA.handle();
}

void reconnect() {
  while (!client.connected()) {
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    ArduinoOTA.handle();
    if (client.connect(clientId.c_str(), "openhabian", "openhabian")) {
      client.subscribe("/home/espsecond/lefteye/command");
      client.subscribe("/home/espsecond/righteye/command");
      client.subscribe("/home/espsecond/peltier/command");
      client.subscribe("/home/espsecond/buzzer/command");
      client.subscribe("/home/espsecond/colorpick/command");
      client.subscribe("/home/espsecond/ring/command");
      client.subscribe("/home/espsecond/square/command");
      client.subscribe("/home/espsecond/ws/command");
    } else {
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String pay = "";
  for (int i = 0; i < length; i++) {
    pay += (char)payload[i];
  }

  if(String(topic) == "/home/espsecond/lefteye/command"){
      if(pay == "1"){
        digitalWrite(leftled, LOW);
      }
      else if(pay == "0"){
        digitalWrite(leftled, HIGH);
      }
  }

  if(String(topic) == "/home/espsecond/righteye/command"){
      if(pay == "1"){
        digitalWrite(rightled, LOW);
      }
      else if(pay == "0"){
        digitalWrite(rightled, HIGH);
      }
  }

  if(String(topic) == "/home/espsecond/peltier/command"){
      if(pay == "1"){
        digitalWrite(peltier, LOW);
      }
      else if(pay == "0"){
        digitalWrite(peltier, HIGH);
      }
  }
  if(String(topic) == "/home/espsecond/buzzer/command"){
    int val = pay.toInt();
    if(val < 150){
      noTone(buzzer);
    }
    else{
      tone(buzzer,val);
    }
  }

  if(String(topic) == "/home/espsecond/colorpick/command"){
    char mes[14] = "";
    pay.toCharArray(mes, sizeof(pay));
    char* token = strtok(mes, ", ");
    int i = 0;
    while (token != NULL) { 
       if(i == 0){
          red = atoi(token);
          i++;
       }else
        if(i == 1){
          green = atoi(token);
          i++;
       }else
       if(i == 2){
          blue = atoi(token);
          i++;
       }
       token = strtok(NULL, ", "); 
    }
    if(ron == 1){
      for(int i = 0; i < 16; i++) 
          rings[i] = CRGB(red, green, blue);
      FastLED.show();
    } 
    if(son == 1){
      for(int i = 0; i < 16; i++) 
          squares[i] = CRGB(red, green, blue);
      FastLED.show();
    } 
    if(won == 1){
      for(int i = 0; i < 58; i++) 
          wss[i] = CRGB(red, green, blue);
      FastLED.show();
    } 
  }

  if(String(topic) == "/home/espsecond/ring/command"){
      if(pay == "1"){
        for(int i = 0; i < 16; i++) 
          rings[i] = CRGB(red, green, blue);
        FastLED.show();
        ron = 1;
      }
      else if(pay == "0"){
        for(int i = 0; i < 16; i++) 
          rings[i] = CRGB(0, 0, 0);
        FastLED.show();
        ron = 0;
      }
  }

  if(String(topic) == "/home/espsecond/square/command"){
      if(pay == "1"){
        for(int i = 0; i < 16; i++) 
          squares[i] = CRGB(red, green, blue);
        FastLED.show();
        son = 1;
      }
      else if(pay == "0"){
        for(int i = 0; i < 16; i++) 
          squares[i] = CRGB(0, 0, 0);
        FastLED.show();
        son = 0;
      }
  }

  if(String(topic) == "/home/espsecond/ws/command"){
      if(pay == "1"){
        for(int i = 0; i < 58; i++) 
          wss[i] = CRGB(red, green, blue);
        FastLED.show();
        won = 1;
      }
      else if(pay == "0"){
        for(int i = 0; i < 58; i++) 
          wss[i] = CRGB(0, 0, 0);
        FastLED.show();
        won = 0;
      }
  }
}
