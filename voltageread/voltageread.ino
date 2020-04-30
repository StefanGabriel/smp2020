#include <SoftwareSerial.h>
SoftwareSerial esp8266(2, 3);
#define NUM_SAMPLES 5

int sum1 = 0;
int sum2 = 0; 
unsigned char sample_count = 0;
float voltage1 = 0.0;           
float voltage2 = 0.0; 

int mVperAmp = 100;
int RawValue1 = 0;
int RawValue2 = 0;
int ACSoffset = 2500; 
double Voltage1 = 0;
double Voltage2 = 0;
double Amps1 = 0;
double Amps2 = 0;

int v1;
int v2;
int v3;
int v4;

String a5;
String a12;
String v5;
String v12;

void setup()
{
    Serial.begin(9600);
    esp8266.begin(9600);
}

void loop()
{
    while (sample_count < NUM_SAMPLES) {
        sum1 += analogRead(A1);
        sum2 += analogRead(A0);
        RawValue1 += analogRead(A3);
        RawValue2 += analogRead(A2);
        sample_count++;
        delay(10);
    }
    
    Voltage1 = ((float)RawValue1/(float)NUM_SAMPLES / 1024.0) * 5000;
    Amps1 = ((Voltage1 - ACSoffset) / mVperAmp);

    Voltage2 = ((float)RawValue2/(float)NUM_SAMPLES / 1024.0) * 5000;
    Amps2 = ((Voltage2 - ACSoffset) / mVperAmp);

    voltage1 = ((float)sum1/(float)NUM_SAMPLES * 5.015) / 1024.0*2.03;
    voltage2 = ((float)sum2/(float)NUM_SAMPLES * 5.015) / 1024.0*4.315;
    
    v1 = (int)(Amps1*100);
    v2 = (int)(Amps2*100);
    v3 = (int)(voltage1*100);
    v4 = (int)(voltage2*100);
    
    a5 = '1' + (String)(v1) + ',';
    a12 = '2' + (String)(v2) + ',';
    v5 = '3' + (String)(v3) + ',';
    v12 = '4' + (String)(v4) + ',';

    char* cstr1 = new char [a5.length()+1];
    char* cstr2 = new char [a12.length()+1];
    char* cstr3 = new char [v5.length()+1];
    char* cstr4 = new char [v12.length()+1];
    strcpy(cstr1, a5.c_str());
    strcpy(cstr2, a12.c_str());
    strcpy(cstr3, v5.c_str());
    strcpy(cstr4, v12.c_str());
    
    esp8266.write(cstr1);
    delay(600);
    esp8266.write(cstr2);
    delay(600);
    esp8266.write(cstr3);
    delay(600);
    esp8266.write(cstr4);
    delay(600);
    
    sample_count = 0;
    sum1 = 0;
    sum2 = 0;
    RawValue1 = 0;
    RawValue2 = 0;

    delete cstr1;
    delete cstr2;
    delete cstr3;
    delete cstr4;
}
