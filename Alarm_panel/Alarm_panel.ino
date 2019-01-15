#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiServer.h>
#include <WiFiServerSecure.h>
#include <WiFiUdp.h>
#include <Keypad.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <PubSubClient.h>
#include <Config.ino>
/*
 * LCD
 * D1 SCL
 * D2 SDA
 * 
 * Number pad
 * A0 Read
 */
 
//Wifi
const char ssid[] = "";        // SSID of wifi network
const char password[] = "";   // Password of wifi netowrk
WiFiClient espClient;

//MQTT
const char mqtt_server[] = "";                   // MQTT server ip
const int mqtt_port = 1883;                      // MQTT server port   default: 1883
const char mqtt_user[] = "";                     // MQTT username
const char mqtt_password[] = "";                 // MQTT user password
const char device_id[] = "";                     // MQTT device name
PubSubClient client(espClient);
char msg[64];
char topic[32];
int a = 0;

//Keypad
int thresholds[12] = {252,170,33,534,485,403,719,684,631,847,822,786};  // Value of the measured voltage for each key.
char keypad[12] = {'1','2','3','4','5','6','7','8','9','*','0','#'};    // Key mapping

//Password
#define Password_Length 5                                       // Length of alarm code + 1 
char Data[Password_Length]; 
char Master[Password_Length] = "";                              // Primary Alarm code
char firstPassword[Password_Length] = "";                       // Secondary Alarm code
byte data_count = 0, master_count = 0;
bool Pass_is_good;
bool armed;

//Lcd
LiquidCrystal_I2C lcd(0x3F, 16, 2);                             //LCD properties LCD address, colums, rows
char display[16];
int timer = 0;
bool display_state = true;

void clearData()
{
  while(data_count !=0)
  {
    Data[data_count--] = 0; 
  }
  return;
}

void callback(char* topic, byte* payload, unsigned int length) 
{
  memcpy(msg, payload, length);
  msg[length] = '\0';
  
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.print(msg);

  if(strcmp(msg,"armed_away") == 0)
  { 
    snprintf(display,16,"Armed!");
    armed = true;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(display);  
  }
  else if(strcmp(msg,"disarmed") == 0)
  {
    snprintf(display,16,"Disarmed!");
    armed = false;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(display);
  }
  else if(strcmp(msg,"pending") == 0)
  {
    snprintf(display,16,"Arming!");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(display);
    delay(10000);
  }

}

void mqttLoop() {
  while (!client.connected()){
    Serial.print("Attempting to connect to MQTT broker at ");
    Serial.print(mqtt_server);
    Serial.print(":");
    Serial.print(mqtt_port);
    

    if(client.connect(device_id)){

      Serial.println("Connected to MQTT broker");
      client.subscribe("home/alarm");
      
    }
    else {
      Serial.print("Connection to MQTT Server failed, rc=");
      Serial.print(client.state());
      Serial.print(" trying again in 5 seconds");

      delay(5000);
    }
  }

  client.loop();
}

void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.hostname("Alarm");
  WiFi.begin(ssid, password);
  lcd.init(); 
  lcd.on();
  lcd.backlight();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
 
  // Print the IP address
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  while (!client.connected()) 
  {
    Serial.println("Connecting to MQTT...");
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password )) 
    {
      Serial.println("connected");
      client.subscribe("home/alarm");  
    } 
    else 
    {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  lcd.setCursor(0,1);
  lcd.print("Code:");
  int value = analogRead(A0);
  if(timer == 2000)
  {
    lcd.off();
    lcd.noBacklight();
    display_state = false;
  }
  else if( timer < 2000)
  {
    timer++;
  }
  
  for (int i = 0; i < 12; i++)
  {
    
    if(abs(value - thresholds[i]) < 5 && display_state && i != 11 && i != 9)    // Read and enter the number pressed to the alarm code
    {
      timer = 0;
      Data[data_count] = keypad[i]; 
      lcd.setCursor(data_count + 5,1); 
      lcd.print(Data[data_count]); 
      data_count++; 
      while( analogRead(A0) < 1000)
      {
        delay(100);
      }
    }
    else if(abs(value - thresholds[11]) < 5 && !display_state)                  // Wake screen if dead with #
    {
      timer = 0;
      lcd.on();
      lcd.backlight();
      display_state = true;
      while( analogRead(A0) < 1000)
      {
        delay(100);
      }
      delay(500);
    }
    else if(abs(value - thresholds[9]) < 5 && display_state)                    // Clear code with *
      timer = 0;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(display);
      clearData();  
    }
  }
  if(data_count == Password_Length-1)                                           // Check if code matches one of the passwords
  {
    lcd.clear();

    if(!strcmp(Data, Master) || !strcmp(Data, firstPassword))
    {
      lcd.print("Correct");
      if(armed)
      {
        snprintf(topic, 32, "home/alarm/set");
        snprintf(msg, 64, "DISARM", device_id);
        client.publish(topic,msg);
      }
      else
      {
        snprintf(topic, 32, "home/alarm/set");
        snprintf(msg, 64, "ARM_AWAY", device_id);
        client.publish(topic,msg);
      }
    }
    else
    {
      lcd.print("Incorrect");
      delay(1500);
    }
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(display);
    clearData();  
  }

  mqttLoop();
}
