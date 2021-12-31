
#define USE_ARDUINO_INTERRUPTS true //Set-up low-level interrupts for most acurate BPM math.
#define ONE_WIRE_BUS 5 //Pair physical data wire from temp sensor to digital PIN 5 on the Arduino
#define WIFI_AP "S10" //Your local wifi network name
#define WIFI_PASSWORD "11234567" //Your local wifi network password
#define TOKEN "QR1YFX0cDow4ppS8PPKW" //Token for your thingsboard server

#include <PulseSensorPlayground.h> //Includes library for using the heart-rate sensor
#include <OneWire.h> //Includes library to enable data processing from temp sensors data line.
#include <DallasTemperature.h> //Includes library to process data from temp sensor and calculate temperture.
#include <ThingsBoard.h>
#include <WiFiEsp.h>
#include <WiFiEspUdp.h>
#include <WiFiEspServer.h>
#include <WiFiEspClient.h> //Libraries for the ESP8266 WiFi module
//#include <PubSubClient.h> //Library for publish/subscripe messaging using MQTT
#include <SoftwareSerial.h> //Library to use digital pins as i/o


PulseSensorPlayground pulseSensor;  //Creates an instance of the PulseSensorPlayground object called "pulseSensor"
OneWire onewire(ONE_WIRE_BUS); //Creates a OneWire instance
DallasTemperature tempSensor(&onewire); //Pass OneWire reference to Dallas Temperature.
WiFiEspClient espClient; // Initialize the Ethernet client object
ThingsBoard thingsBoard(espClient); //Pass ethernet object to thingsboard
SoftwareSerial soft(2, 3); // RX, TX

const int PulseWire = 0; //PulseSensor data wire connected to ANALOG PIN 0
int Threshold = 515; //Raw output from pluse sensor sensor used to determine the minimum out for an actual heartbeat
char thingsboardServer[] = "demo.thingsboard.io"; //Server address for thingsboard
int status = WL_IDLE_STATUS;
unsigned long lastSend;

void setup() {
  Serial.begin(9600); //For Serial Monitor
  InitWiFi();
  pulseSensor.analogInput(PulseWire); //Assigns the analog input to the pulseSensor object.
  pulseSensor.setThreshold(Threshold); //Assigns our selected threshold value to the pulseSensor object.
  pulseSensor.begin(); //Start pulse sensor
  tempSensor.begin(); //Start temp sensor 
  lastSend = 0;
}

void loop() {
  status = WiFi.status();
  if ( status != WL_CONNECTED) {
    while ( status != WL_CONNECTED) {
      // Connect to WPA/WPA2 network
      status = WiFi.begin(WIFI_AP, WIFI_PASSWORD);
      delay(500);
    }
    Serial.println("Connected to AP");
  }

  if ( !thingsBoard.connected() ) {
    reconnect();
  }

  if ( millis() - lastSend > 1000 ) { // Update and send only after 1 seconds
    tempandbpm();
    lastSend = millis();
  }
  thingsBoard.loop();
}


void tempandbpm() {
  
  int myBPM = pulseSensor.getBeatsPerMinute();  // Calls function on our pulseSensor object that returns BPM as an int.
  
  tempSensor.requestTemperatures();
  int myTEMP = tempSensor.getTempCByIndex(0); // Calls function on our tempSensor object that returns temp as an int.
 
  Serial.print("BPM: ");
  Serial.print(myBPM);
  Serial.print(" || Temp: ");
  Serial.print(myTEMP);
  Serial.print("\n");

  thingsBoard.sendTelemetryFloat("Temperature", myTEMP); //Pass temp data to MQTT broker
  thingsBoard.sendTelemetryFloat("BPM", myBPM); // //Pass pulse data to MQTT broker
 }

 void InitWiFi()
{
  // initialize serial for ESP module
  soft.begin(9600);
  // initialize ESP module
  WiFi.init(&soft);
  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  Serial.println("Connecting to AP ...");
  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(WIFI_AP);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(WIFI_AP, WIFI_PASSWORD);
    delay(500);
  }
  Serial.println("Connected to AP");
}

void reconnect() {
  // Loop until we're reconnected
  while (!thingsBoard.connected()) {
    Serial.print("Connecting to ThingsBoard node ...");
    // Attempt to connect (clientId, username, password)
    if ( thingsBoard.connect(thingsboardServer, TOKEN) ) {
      Serial.println( "[DONE]" );
    } else {
      //Serial.print( "[FAILED]" );
      //Serial.println( " : retrying in 5 seconds" );
      // Wait 5 seconds before retrying
      delay(10);
    }
  }
}
