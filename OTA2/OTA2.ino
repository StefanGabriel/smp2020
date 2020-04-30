#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <FastLED.h>
#include <SimpleDHT.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include "Ucglib.h"
#include "Nextion.h"

#define dht 12
#define fan 0
#define motor 16

SimpleDHT11 dht11;
WiFiClient espClient;
PubSubClient client(espClient);
LiquidCrystal_I2C lcd_green(0x26, 16, 2);
LiquidCrystal_I2C lcd_blue(0x27, 16, 2);
Ucglib_ILI9341_18x240x320_HWSPI ucg(/*cd=*/ 15, /*cs=*/ 10, /*reset=*/ 2);

const char* ssid = "Sagemcom 2.4 GHz";
const char* password = "............";
const char* mqtt_server = "192.168.0.33";

int tmp = 0, hum = 0;
int tmp_prev = 0, hum_prev = 0;
byte temperature = 0;
byte humidity = 0;

unsigned long currentMillis = 60001;
unsigned long previousMillis = 0;
unsigned long interval = 1500;

String IncomingString="";
boolean StringReady = 0;

String voltage5 = "0.0";
String voltage12 = "";
String amperage5 = "0.0";
String amperage12 = "";
String lefteye = "Off";
String righteye = "Off";
String peltier = "Off";
String buzzer = "0";
String frontring = "Off";
String square = "Off";
String cases = "Off";
String motors = "Off";
String fans = "Off";

String voltage5_prev = "";
String voltage12_prev = "";
String amperage5_prev = "";
String amperage12_prev = "";
String lefteye_prev = "Off";
String righteye_prev = "Off";
String peltier_prev = "Off";
String buzzer_prev = "0";
String frontring_prev = "Off";
String square_prev = "Off";
String cases_prev = "Off";
String motors_prev = "Off";
String fans_prev = "Off";
char message[10];

NexText t5v = NexText(0, 12, "t8"); 
NexText t5a = NexText(0, 13, "t9"); 
NexText t12v = NexText(0, 14, "t10"); 
NexText t12a = NexText(0, 15, "t11"); 
NexText thum = NexText(0, 16, "t12"); 
NexText ttmp = NexText(0, 17, "t13"); 

NexButton leyeon = NexButton(1, 5, "b0");
NexButton leyeoff = NexButton(1, 9, "b1");
NexButton reyeon = NexButton(1, 10, "b4");
NexButton reyeoff = NexButton(1, 11, "b5");
NexButton peltieron = NexButton(1, 12, "b6");
NexButton peltieroff = NexButton(1, 13, "b7");
NexButton motoron = NexButton(1, 14, "b8");
NexButton motoroff = NexButton(1, 15, "b9");

NexButton fanon = NexButton(2, 5, "b0"); 
NexButton fanoff = NexButton(2, 9, "b1"); 
NexButton ringon = NexButton(2, 10, "b4"); 
NexButton ringoff = NexButton(2, 11, "b5"); 
NexButton squareon = NexButton(2, 12, "b6"); 
NexButton squareoff = NexButton(2, 13, "b7"); 
NexButton caseon = NexButton(2, 14, "b8"); 
NexButton caseoff = NexButton(2, 15, "b9"); 
NexButton buzzon = NexButton(2, 17, "b10"); 
NexButton buzzoff = NexButton(2, 18, "b11"); 

NexTouch *nex_listen_list[] = {
  &leyeon,
  &leyeoff,
  &reyeon,
  &reyeoff,
  &peltieron,
  &peltieroff,
  &motoron,
  &motoroff,
  &fanon,
  &fanoff,
  &ringon,
  &ringoff,
  &squareon,
  &squareoff,
  &caseon,
  &caseoff,
  &buzzon,
  &buzzoff,
  NULL
};

void leyeonPopCallback(void *ptr) {
  lefteye = "On ";
  client.publish("/home/espsecond/lefteye/state", "ON");
}

void reyeonPopCallback(void *ptr) {
  righteye = "On ";
  client.publish("/home/espsecond/righteye/state", "ON");
}

void peltieronPopCallback(void *ptr) {
  peltier = "On ";
  client.publish("/home/espsecond/peltier/state", "ON");
}

void motoronPopCallback(void *ptr) {
  motors = "On ";
  digitalWrite(motor, LOW);
  client.publish("/home/esp8266/motor/state", "ON");
}

void fanonPopCallback(void *ptr) {
  fans = "On ";
  digitalWrite(fan, LOW);
  client.publish("/home/esp8266/fan/state", "ON");
}

void ringonPopCallback(void *ptr) {
  frontring = "On ";
  client.publish("/home/espsecond/ring/state", "ON");
}

void squareonPopCallback(void *ptr) {
  square = "On ";
  client.publish("/home/espsecond/square/state", "ON");
}

void caseonPopCallback(void *ptr) {
  cases = "On ";
  client.publish("/home/espsecond/ws/state", "ON");
}

void buzzonPopCallback(void *ptr) {
  buzzer = "On ";
  client.publish("/home/espsecond/buzzer/state", "1000");
}

void leyeoffPopCallback(void *ptr) {
  lefteye = "Off";
  client.publish("/home/espsecond/lefteye/state", "OFF");
}

void reyeoffPopCallback(void *ptr) {
  righteye = "Off";
  client.publish("/home/espsecond/righteye/state", "OFF");
}

void peltieroffPopCallback(void *ptr) {
  peltier = "Off";
  client.publish("/home/espsecond/peltier/state", "OFF");
}

void motoroffPopCallback(void *ptr) {
  motors = "Off";
  digitalWrite(motor, HIGH);
  client.publish("/home/esp8266/motor/state", "OFF");
}

void fanoffPopCallback(void *ptr) {
  fans = "Off";
  digitalWrite(fan, HIGH);
  client.publish("/home/esp8266/fan/state", "OFF");
}

void ringoffPopCallback(void *ptr) {
  frontring = "Off";
  client.publish("/home/espsecond/ring/state", "OFF");
}

void squareoffPopCallback(void *ptr) {
  square = "Off";
  client.publish("/home/espsecond/square/state", "OFF");
}

void caseoffPopCallback(void *ptr) {
  cases = "Off";
  client.publish("/home/espsecond/ws/state", "OFF");
}

void buzzoffPopCallback(void *ptr) {
  buzzer = "Off";
  client.publish("/home/espsecond/buzzer/state", "50");
}


void setup() 
{
  Serial.begin(9600);
  Serial.setTimeout(100);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(5000);
    ESP.restart();
  }
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  nexInit();
  pinMode(fan, OUTPUT);
  pinMode(motor, OUTPUT);

  digitalWrite(fan, HIGH);
  digitalWrite(motor, HIGH);

  lcd_green.init();
  lcd_green.backlight();
  lcd_green.setCursor(0, 0);
  lcd_green.print("0000");
  lcd_green.clear();
  lcd_green.setCursor(0, 0);
  lcd_green.print("Temperature ");
  lcd_green.print("00");
  lcd_green.print((char)223);
  lcd_green.print("C");
  lcd_green.setCursor(0, 1);
  lcd_green.print("Humidity     ");
  lcd_green.print("00");
  lcd_green.print("%");

  lcd_blue.init();
  lcd_blue.backlight();
  lcd_blue.setCursor(0, 0);
  lcd_blue.print("0000");
  lcd_blue.clear();
  lcd_blue.setCursor(0, 0);
  lcd_blue.print(" 5V  ");
  lcd_blue.print("0.00");
  lcd_blue.print("V ");
  lcd_blue.print("0.00");
  lcd_blue.print("A");
  lcd_blue.setCursor(0, 1);
  lcd_blue.print("12V ");
  lcd_blue.print("00.00");
  lcd_blue.print("V ");
  lcd_blue.print("0.00");
  lcd_blue.print("A");

  //ucg.begin(UCG_FONT_MODE_TRANSPARENT);
  ucg.begin(UCG_FONT_MODE_SOLID);
  ucg.clearScreen();
  ucg.setRotate90();
  
//  t5v.attachPop(t5vPopCallback, &t5v);
//  t5a.attachPop(t5aPopCallback, &t5a);
//  t12v.attachPop(t12vPopCallback, &t12v);
//  t12a.attachPop(t12aPopCallback, &t12a);
//  ttmp.attachPop(ttmpPopCallback, &thum);
  leyeon.attachPop(leyeonPopCallback, &leyeon);
  leyeoff.attachPop(leyeoffPopCallback, &leyeoff);
  reyeon.attachPop(reyeonPopCallback, &reyeon);
  reyeoff.attachPop(reyeoffPopCallback, &reyeoff);
  peltieron.attachPop(peltieronPopCallback, &peltieron);
  peltieroff.attachPop(peltieroffPopCallback, &peltieroff);
  motoron.attachPop(motoronPopCallback, &motoron);
  motoroff.attachPop(motoroffPopCallback, &motoroff);
  fanon.attachPop(fanonPopCallback, &fanon);
  fanoff.attachPop(fanoffPopCallback, &fanoff);
  ringon.attachPop(ringonPopCallback, &ringon);
  ringoff.attachPop(ringoffPopCallback, &ringoff);
  squareon.attachPop(squareonPopCallback, &squareon);
  squareoff.attachPop(squareoffPopCallback, &squareoff);
  caseon.attachPop(caseonPopCallback, &caseon);
  caseoff.attachPop(caseoffPopCallback, &caseoff);
  buzzon.attachPop(buzzonPopCallback, &buzzon);
  buzzoff.attachPop(buzzoffPopCallback, &buzzoff);

  ucg.setFont(ucg_font_courR14_mr);
  ucg.setColor(255, 204, 0);
  ucg.setColor(1, 0, 0, 0);
  ucg.setPrintPos(0,20);
  ucg.print("Left Eye");
  ucg.setPrintPos(0,50);
  ucg.print("Right Eye");
  ucg.setPrintPos(0,80);
  ucg.print("Peltier");
  ucg.setPrintPos(0,110);
  ucg.print("Fan");
  ucg.setPrintPos(0,140);
  ucg.print("Ring");
  ucg.setPrintPos(0,170);
  ucg.print("Square");
  ucg.setPrintPos(0,200);
  ucg.print("Case Leds");
  ucg.setPrintPos(0,230);
  ucg.print("Motor");

  ucg.setColor(0, 0, 255);
  ucg.setPrintPos(110,20);
  ucg.print(lefteye);
  ucg.setPrintPos(110,50);
  ucg.print(righteye);
  ucg.setPrintPos(110,80);
  ucg.print(peltier);
  ucg.setPrintPos(110,110);
  ucg.print(fans);
  ucg.setPrintPos(110,140);
  ucg.print(frontring);
  ucg.setPrintPos(110,170);
  ucg.print(square);
  ucg.setPrintPos(110,200);
  ucg.print(cases);
  ucg.setPrintPos(110,230);
  ucg.print(motors);

  ucg.setColor(255, 204, 0);
  ucg.setPrintPos(160,20);
  ucg.print("Buzzer");
  ucg.setPrintPos(160,50);
  ucg.print("5V Volt");
  ucg.setPrintPos(160,80);
  ucg.print("5V Amps");
  ucg.setPrintPos(160,110);
  ucg.print("12V Volt");
  ucg.setPrintPos(160,140);
  ucg.print("12V Amps");
  ucg.setPrintPos(160,170);
  ucg.print("Temp");
  ucg.setPrintPos(160,200);
  ucg.print("Humidity");

  ucg.setFont(ucg_font_courR14_mr);
  ucg.setColor(0, 0, 255);
  ucg.setPrintPos(250,20);
  ucg.print(buzzer);
  ucg.setPrintPos(300,20);
  ucg.print("Hz");
  ucg.setPrintPos(250,50);
  ucg.print(voltage5);
  ucg.setPrintPos(300,50);
  ucg.print("V");
  ucg.setPrintPos(250,80);
  ucg.print(amperage5);
  ucg.setPrintPos(300,80);
  ucg.print("A");
  ucg.setPrintPos(250,110);
  ucg.print(voltage12);
  ucg.setPrintPos(300,110);
  ucg.print("V");
  ucg.setPrintPos(250,140);
  ucg.print(amperage12);
  ucg.setPrintPos(300,140);
  ucg.print("A");
  ucg.setPrintPos(250,170);
  ucg.print(tmp);
  ucg.setPrintPos(300,170);
  ucg.print("C");
  ucg.setPrintPos(250,200);
  ucg.print(hum);
  ucg.setPrintPos(300,200);
  ucg.print("%");

  ucg.setColor(255, 255, 255);
  ucg.drawVLine(150, 0, 240);
  ucg.setColor(0, 0, 255);
  
  ArduinoOTA.begin();
}

void loop() 
{
    if (!client.connected()) {
      reconnect();
    }
    client.loop();
    
    if(currentMillis - previousMillis > interval) {
          previousMillis = currentMillis;  
          dht11.read(dht, &temperature, &humidity, NULL);
            while(temperature == 0 && humidity == 0){
              dht11.read(dht, &temperature, &humidity, NULL);
          }
          tmp_prev = tmp;
          tmp = temperature;
          hum_prev = hum;
          hum = humidity;
          
          if(tmp != tmp_prev){
            //tmp_prev = tmp;
            ucg.setPrintPos(250,170);
            ucg.print(tmp);
            lcd_green.setCursor(12, 0);
            lcd_green.print(tmp);
            String(tmp).toCharArray(message, 10);
            client.publish("/home/esp8266/temperature", message);
            ttmp.setText(message);
          }
          if(hum != hum_prev){
            //hum_prev = hum;
            ucg.setPrintPos(250,200);
            ucg.print(hum);
            lcd_green.setCursor(13, 1);
            lcd_green.print(hum);
            String(hum).toCharArray(message, 10);
            client.publish("/home/esp8266/humidity", message);
            thum.setText(message);
          }
          
          if(voltage5.equals(voltage5_prev) == false){
            voltage5_prev = voltage5;
            String(voltage5).toCharArray(message, 10);
            t5v.setText(message);
            ucg.setPrintPos(250,50);
            ucg.print(voltage5);
            lcd_blue.setCursor(5, 0);
            lcd_blue.print(voltage5);
          }
          if(voltage12.equals(voltage12_prev) == false){
            voltage12_prev = voltage12;
            String(voltage12).toCharArray(message, 10);
            t12v.setText(message);
            ucg.setPrintPos(250,110);
            ucg.print(voltage12);
            lcd_blue.setCursor(4, 1);
            lcd_blue.print(voltage12);
          }
          if(amperage5.equals(amperage5_prev) == false){
            amperage5_prev = amperage5;
            String(amperage5).toCharArray(message, 10);
            t5a.setText(message);
            ucg.setPrintPos(250,80);
            ucg.print(amperage5);
            lcd_blue.setCursor(11, 0);
            lcd_blue.print(amperage5);
          }
          if(amperage12.equals(amperage12_prev) == false){
            amperage12_prev = amperage12;
            String(amperage12).toCharArray(message, 10);
            t12a.setText(message);
            ucg.setPrintPos(250,140);
            ucg.print(amperage12);
            lcd_blue.setCursor(11, 1);
            lcd_blue.print(amperage12);
          }
          
    }
    currentMillis = millis();
    
    if(lefteye.equals(lefteye_prev) == false){
      lefteye_prev = lefteye;
      ucg.setPrintPos(110,20);
      ucg.print(lefteye);
    }
    if(righteye.equals(righteye_prev) == false){
      righteye_prev = righteye;
      ucg.setPrintPos(110,50);
      ucg.print(righteye);
    }
    if(peltier.equals(peltier_prev) == false){
      peltier_prev = peltier;
      ucg.setPrintPos(110,80);
      ucg.print(peltier);
    }
    if(fans.equals(fans_prev) == false){
      fans_prev = fans;
      ucg.setPrintPos(110,110);
      ucg.print(fans);
    }
    if(frontring.equals(frontring_prev) == false){
      frontring_prev = frontring;
      ucg.setPrintPos(110,140);
      ucg.print(frontring);
    }
    if(square.equals(square_prev) == false){
      square_prev = square;
      ucg.setPrintPos(110,170);
      ucg.print(square);
    }
    if(cases.equals(cases_prev) == false){
      cases_prev = cases;
      ucg.setPrintPos(110,200);
      ucg.print(cases);
    }
    if(motors.equals(motors_prev) == false){
      motors_prev = motors;
      ucg.setPrintPos(110,230);
      ucg.print(motors);
    }
    if(buzzer.equals(buzzer_prev) == false){
      buzzer_prev = buzzer;
      ucg.setPrintPos(250,20);
      ucg.print(buzzer);
      if(buzzer.length() == 3){
        ucg.print(" ");
      } else if(buzzer.length() == 2){
        ucg.print("  ");
      }
    }
    nexLoop(nex_listen_list);
    ArduinoOTA.handle();
}

void reconnect() {
  while (!client.connected()) {
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    ArduinoOTA.handle();
    if (client.connect(clientId.c_str(), "openhabian", "openhabian")) {
      client.subscribe("/home/esp8266/fan/command");
      client.subscribe("/home/esp8266/motor/command");
      client.subscribe("/home/espsecond/5vrail/state");
      client.subscribe("/home/espsecond/12vrail/state");
      client.subscribe("/home/espsecond/5vamp/state");
      client.subscribe("/home/espsecond/12vamp/state");
      client.subscribe("/home/espsecond/lefteye/command");
      client.subscribe("/home/espsecond/righteye/command");
      client.subscribe("/home/espsecond/peltier/command");
      client.subscribe("/home/espsecond/buzzer/command");
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

  if(String(topic) == "/home/esp8266/fan/command"){
      if(pay == "1"){
        digitalWrite(fan, LOW);
        fans = "On ";
      }
      else if(pay == "0"){
        digitalWrite(fan, HIGH);
        fans = "Off";
      }
  }

  if(String(topic) == "/home/esp8266/motor/command"){
      if(pay == "1"){
        digitalWrite(motor, LOW);
        motors = "On ";
      }
      else if(pay == "0"){
        digitalWrite(motor, HIGH);
        motors = "Off";
      }
  }

  if(String(topic) == "/home/espsecond/5vrail/state"){
      voltage5 = pay;
  }

  if(String(topic) == "/home/espsecond/12vrail/state"){
      voltage12 = pay;
  }

  if(String(topic) == "/home/espsecond/5vamp/state"){
      amperage5 = pay;
  }

  if(String(topic) == "/home/espsecond/12vamp/state"){
      amperage12 = pay;
  }

  if(String(topic) == "/home/espsecond/lefteye/command"){
      if(pay == "1"){
        lefteye = "On ";
      }
      else if(pay == "0"){
        lefteye = "Off";
      }
  }

  if(String(topic) == "/home/espsecond/righteye/command"){
      if(pay == "1"){
        righteye = "On ";
      }
      else if(pay == "0"){
        righteye = "Off";
      }
  }

  if(String(topic) == "/home/espsecond/peltier/command"){
      if(pay == "1"){
        peltier = "On ";
      }
      else if(pay == "0"){
        peltier = "Off";
      }
  }
  if(String(topic) == "/home/espsecond/buzzer/command"){
    buzzer = String(pay.toInt());
  }

  if(String(topic) == "/home/espsecond/ring/command"){
      if(pay == "1"){
        frontring = "On ";
      }
      else if(pay == "0"){
        frontring = "Off";
      }
  }

  if(String(topic) == "/home/espsecond/square/command"){
      if(pay == "1"){
        square = "On ";
      }
      else if(pay == "0"){
        square = "Off";
      }
  }

  if(String(topic) == "/home/espsecond/ws/command"){
      if(pay == "1"){
        cases = "On ";
      }
      else if(pay == "0"){
        cases = "Off";
      }
  }
}
