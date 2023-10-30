#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Arduino_JSON.h>
#include <ArduinoJson.h>
////////////////////////////////////////
#define HOST "192.168.100.194"
//#define WIFI_SSID "Me"            // WIFI SSID here                                   
//#define WIFI_PASSWORD "dennis22"  

//////////////////////////////////////////////
long trig= D6;
long echo= D5;
long duration;
long distance;
///////////////////////////////////////////////
#include <SoftwareSerial.h>
SoftwareSerial mySerial(D8, D7); //SIM800L Tx & Rx is connected to Arduino #3 & #2
////////////////////////////////////////////////
// Update HOST URL here
int led=D0;
int pump =D1;
#define WIFI_SSID "A"            // WIFI SSID here                                   
#define WIFI_PASSWORD "1234567890" 

////////////////////////////////////////////////////
String sendval, sendval2, sendval3,  postData, postData2;
//String name = "Dustbin1";
//String location = "It Manager";
/////////////////////////////////////////////////////////

String phoneNumber="0780649018";
  WiFiClient client;
HTTPClient http;  

int i=0;
int j=0;
#define SensorPin A0
///////////////////////////////////////////////////////////////////////////////////

unsigned long int avgValue;  //Store the average value of the sensor feedback
float b;
int buf[10],temp;

void setup()
{
  
  Serial.begin(9600); // Serial Communication for Serial Monitor is starting with 9600 of baudrate speed
   mySerial.begin(9600);
  pinMode(led,OUTPUT);
  pinMode(pump,OUTPUT);
   //callNumber();
   ////////////////////////////////////////////
    pinMode(trig,OUTPUT);
    pinMode(echo,INPUT);
///////////////////////////////////////////////////////////////////
digitalWrite(pump, HIGH);

 while (!mySerial.available()) {
    mySerial.println("AT");
    delay(1000);
    Serial.println("connecting....");
  }
  Serial.println("Connected..");

  delay(1000);
  /////////////////////////////////////////////////////
  ////WiFi.mode(WIFI_STA);           
WiFi.begin(WIFI_SSID, WIFI_PASSWORD);                                     //try to connect with wifi
Serial.print("Connecting to ");
Serial.print(WIFI_SSID);
while (WiFi.status() != WL_CONNECTED) 
{ Serial.print(".");
    delay(500); }

Serial.println();
Serial.print("Connected to ");
Serial.println(WIFI_SSID);
Serial.print("IP Address is : ");
Serial.println(WiFi.localIP());    //print local IP address
delay(30);

///////////////////////////////////////////////////////

}

void loop()
{


///////////////////////////////////////////////////////////////////////////////////
  digitalWrite(trig,LOW);
  delayMicroseconds(2);
  digitalWrite(trig,HIGH);
  delayMicroseconds(10);
  digitalWrite(trig,LOW);
  duration = pulseIn(echo,HIGH);
  distance=duration*0.017;
  Serial.print("Distance:   ");
  Serial.println(distance);
  Serial.print("");

if(distance<=6){
  if(i==0){
   digitalWrite(led,LOW);
   digitalWrite(pump,HIGH);
   Serial.println("Tank is full");
  //sendSms("Tank is Full");
   i=1;
  }
  else{
    i=0;
    }
}
else{
  
  if(j==0){
   digitalWrite(led,HIGH);
   digitalWrite(pump,LOW);
   Serial.println("Tank is NOt full Pump is HIGHT");
   //sendSms("Tank is not Full");
   j=1;
  }
  else{
    j=0;
    }
}
long FinalLength=distance-7;
String water=String(FinalLength);
 Serial.println(water);
ph(water);
}
void http1(String value1, String Value2) {
  String sendval = String(value1);
  String sendval2 = String(Value2);

  if (WiFi.status() == WL_CONNECTED) {
    BearSSL::WiFiClientSecure client;
    client.setInsecure(); // Ignore SSL certificate validation

    HTTPClient https;
    https.begin(client, "https://tank.rwandahouseland.com/insert.php"); // HTTPS
    https.setTimeout(3000);

    // Create a JSON object to hold the data
    StaticJsonDocument<200> jsonDoc;
    jsonDoc["sendval"] = sendval;
    jsonDoc["sendval2"] = sendval2;
    String jsonStr;
    serializeJson(jsonDoc, jsonStr);

    // Set the Content-Type header to application/json
    https.addHeader("Content-Type", "application/json");

    int httpCode = https.POST(jsonStr);

    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = https.getString();
        Serial.println(payload);
      }
    }
    else {
      Serial.print("HTTP POST request failed with error code: ");
      Serial.println(httpCode);
    }

    https.end();
    delay(1000);
  }
}


void sendSms( String text) {
  mySerial.println("AT");
 updateSerial();
  mySerial.println("AT+CMGF=1");
  updateSerial();
  mySerial.print("AT+CMGS=\"");
  updateSerial();
  mySerial.print(phoneNumber);
  updateSerial();
  mySerial.println("\"");
 updateSerial();
  mySerial.print(text);
updateSerial();
  mySerial.write(26);
  Serial.println("SMS sent to " + phoneNumber);
  delay(1000);
}


  void updateSerial()
{
  delay(500);
  while (Serial.available()) 
  {
    mySerial.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while(mySerial.available()) 
  {
    Serial.write(mySerial.read());//Forward what Software Serial received to Serial Port
  }
}

void ph(String ph){
  
  String ph_data;
  for(int i=0;i<10;i++)       //Get 10 sample value from the sensor for smooth the value
  { 
    buf[i]=analogRead(SensorPin);
    delay(10);
  }
  for(int i=0;i<9;i++)        //sort the analog from small to large
  {
    for(int j=i+1;j<10;j++)
    {
      if(buf[i]>buf[j])
      {
        temp=buf[i];
        buf[i]=buf[j];
        buf[j]=temp;
      }
    }
  }
  avgValue=0;
  
  for(int i=2;i<8;i++)                      //take the average value of 6 center sample
    avgValue+=buf[i];
  float phValue=(float)avgValue*5.0/1024/6; //convert the analog into millivolt
  phValue=3.5*phValue-3;                      //convert the millivolt into pH value
  Serial.print("    pH:");  
  Serial.print(phValue,2);
  Serial.println(" ");
  //digitalWrite(13, HIGH);       
  delay(100);
  //digitalWrite(13, LOW); 

  if(phValue<7.49 ){
 
   ph_data="NOT CLEAN";
   sendSms("Water is not clean");
     http1(ph,ph_data);
  }
  else if(phValue>7.51){
      ph_data=" CLEAN";
        http1(ph,ph_data);
    }
//    else{
//        ph_data="CLEAN";
//          http1(ph,ph_data);
//      }
  

  }
