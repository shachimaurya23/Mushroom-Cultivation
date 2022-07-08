#include <Wire.h>
#include <BH1750.h>
#include <DHT.h>
#include<ThingSpeak.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

WiFiClient client;
#define DHTPIN 4     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11

DHT dht(DHTPIN, DHTTYPE);    //initializing DHT11 sensor
BH1750 lightMeter;     //initializing BH1750 light intensity sensor

//ThingSpeak connection
const char* write_api="RRTFGPHJ654TATVV";
#define SECRET_CH_ID 1562729;
unsigned long myChannelNumber = SECRET_CH_ID;
const char *server="api.thingspeak.com";

const char *ssid = "App";      //wifiname
const char *password = "shachi23";    //password

int count = 0;      //cultivation mode count

int lightHigh = 300;   //unit lux for [BUZZ Alert]
int lightLow = 50;    //unit lux for [BUZZ Alert]

int tempHigh = 29;     //unit oC for [FAN]
int tempLow = 26;      //unit oC for [FAN]

int humidityHigh = 90;    //unit % RH for [AirPump]
int humidityLow = 80;     //unit % RH for [AirPump]

//**connect to the WiFi**
void connectToWiFi(){
  WiFi.mode(WIFI_OFF);     //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);     //connecting to wifi
  
  while (WiFi.status() != WL_CONNECTED)  //when trying to connect to wifi
  {  
  delay(500);
  Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("Connected");      // successfully connected to wifi
  
  Serial.print(F("Connected \n"));     //not connected
  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); //IP address assigned to ESP

}

void setup()     //to connect to the sensors and actuators
{

  Serial.begin(115200);  //upload speed

  pinMode(D5, OUTPUT); //BUZZER 
  pinMode(D6, OUTPUT); //AIR PUMP
  pinMode(D1, OUTPUT); //FAN
  pinMode(D7, INPUT); //BUTTON
  
  Serial.println(F("DHT11 test!"));      //DHT11 begin 
  dht.begin();
  Wire.begin(D3, D4);         //LightPin start
  lightMeter.begin(); 
  Serial.println(F("BH1750 Advanced begin"));
 
  
    connectToWiFi();
}

//to switch mode and indicate it with buzzer
void cultivationMode(){
  int pushb=digitalRead(D7);    //push button
  if(pushb==HIGH){
    count++;
    if(count == 0){   //Oyster Mushroom
      digitalWrite(D5, HIGH); //BUZZ
      delay(200);
      digitalWrite(D5, LOW); //BUZZ
      
      
    }
    else if(count == 1){   //White Button Mushroom
      digitalWrite(D5, HIGH); //BUZZ
      delay(200);
      digitalWrite(D5, LOW); //BUZZ
      delay(200);
      digitalWrite(D5, HIGH); //BUZZ
      delay(200);
      digitalWrite(D5, LOW); //BUZZ
      
    }
    
    else{
      count = 0; //When button is presssed after white button mushroom
    }
    
  }
  else{
    digitalWrite(D5, LOW); //BUZZ
  } 
  
}

//actuator working logic 
void Logic(float temp,float humi,float lx){

  updatep();//get threshold of mushroom depending on its type

  if(temp > tempHigh && humi > humidityHigh){

    digitalWrite(D6, LOW);    // air pump off   
    digitalWrite(D1, HIGH);   // FAN on   
    
  }else if(temp > tempHigh && humi < humidityLow){

    digitalWrite(D6, HIGH);    // air pump on   
    digitalWrite(D1, HIGH);   // FAN on   
  }else if(temp < tempLow && humi > humidityLow){

    digitalWrite(D6, LOW);    // air pump off   
    digitalWrite(D1, HIGH);   //FAN off   
  }else if(humi < humidityLow){

    digitalWrite(D6, HIGH);    // air pump on   
    digitalWrite(D1, HIGH);   //FAN on   
  }else if(humi > humidityHigh){

    digitalWrite(D6, LOW);    // air pump off
    digitalWrite(D1, HIGH);   // FAN on   
  }else if(temp > tempHigh){

    digitalWrite(D6, LOW);    // air pump off   
    digitalWrite(D1, HIGH);   // FAN on   
  }
  else{
    digitalWrite(D6, LOW);    // air pump off  
    digitalWrite(D1, LOW);    // FAN off  
  }
  
}


//read and print the sensor data and pass it to ThingSpeak channel
void loop() {
  
  // Wait a few seconds between measurements.
  delay(2000);
  float lux = lightMeter.readLightLevel();        // read from BH1750 sensor the light intensity
  
    //check if there's a connection to WiFi or not
    if(WiFi.status() != WL_CONNECTED)
    {
      connectToWiFi();
    }
    
  cultivationMode();       //check the mode of mushroom currently in
  

  
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  
  //print the data of sensors to the serial monitor
  Serial.print(F("\nHumidity: "));
  Serial.print(h);
  
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  
  Serial.print(F("°C "));
  
  Serial.print(f);
  Serial.print(F("°F "));
  
  Serial.print("  Light: ");
  Serial.print(lux);
  Serial.println(" lx");
  delay(1000);
  
  Serial.print("index: ");
  Serial.print(hic);
  Serial.print(F("°C "));
  
  Serial.print(hif);
  Serial.println(F("°F "));

  Serial.print(lux);
  Serial.println(F("lx "));

  //ThingSpeak Write data
  ThingSpeak.setField(1,h);
  ThingSpeak.setField(2,t);
  ThingSpeak.setField(3,f);
  ThingSpeak.setField(4,lux);
  ThingSpeak.setField(5,hic);
  ThingSpeak.setField(6,hif);
  
int x=ThingSpeak.writeFields(myChannelNumber,write_api);        //ulpoading the data to ThingSpeak
if(x == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
 
  Logic(t,h,lux);  //check the data to control the actuators i.e. fan and air pump

 //when light intensity is too high sound alarm
  if(lux >= 300){ 
        
       Serial.println("light high");
      digitalWrite(D5, HIGH);   // buzz & air pump on
      delay(1000);              // wait for a second
      digitalWrite(D5, LOW);    // buzz & air pump off
      }
   //when light intensity has returned to normal   
  else{
    digitalWrite(D6, LOW);    // air pump off  
    digitalWrite(D1, LOW);    // air pump off  
  }
  

}

// to compare the value to temperature and humidity for the mushroom depending on its type
void updatep(){

 if(count == 0){
    Serial.print("Oyster mushroom");

    tempLow = 18;
    tempHigh = 24;

    humidityLow = 85;
    humidityHigh = 90;
    
  }else if(count == 1){
    
    Serial.print("White button mushroom");

    tempLow = 26;
    tempHigh = 29;

    humidityLow = 80;
    humidityHigh = 90;
  }
}
