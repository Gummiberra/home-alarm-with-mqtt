# home-alarm-with-mqtt

## Introduction
This is a simple home alarm system using MQTT and home assistant to arm/de-arm alarm in home assistant.

## Getting Started
1.	Requirements
2.	Installation process
3.	Build

### 1 Requirements
1x Wemos D1 mini  
1x LCD1602  
1x 3x4 keypad  
3x 1.5k ohm resistors  
3x 390 ohm resistors  
1x 4.7k ohm resistor  

### 2 Installation process
Use this circuit diagram
![GitHub Logo](/Alarm.jpg)

Measure the thresholds of each keypad key to set the thresholds for the application since it uses analog input.

### 3 Build
Edit the following in the Alarm_panel.ino file
1.  SSID should be set to your wifi's SSID
2.  Password should be set to match the wifi's password
3.  Mqtt_server should be the ip of the mqtt server used
4.  Mqtt_user should be an mqtt user
5.  Mqtt_password should match the password of selected mqtt user
6.  Device_id should is the name this device will get
7.  Edit the thresholds for each key to match the once you measured
8.  Define a master password and a secondary password

When you've edited the following just flash the D1 with the code and plug it in.  
Happy tinkering!

## Support
If there is any questions or bugs regarding this product contact me at:  
<simon@simonlarsson.dev>

## Maintainers
- Simon Larsson

## Authors and acknowledgment
- Simon Larsson

## Tools
- Arduino IDE
- Home Assistant
- Draw.io
