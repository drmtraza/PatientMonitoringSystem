# PatientMonitoringSystem

This project is to provide real time updates to a patients vital using Iot.
Specifically, an Arduino UNO is used to get data from a pulse sensor and a temprature sensor. Which is then passed onto the an ESP8266-01 WiFi module, to be sent to a MQTT broker and accessed by the end user.

NOTE: The WiFiEsp library has been modified, as it would frequently timeout while trying to connect to the network or mqtt broker.
