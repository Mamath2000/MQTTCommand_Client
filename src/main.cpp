#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>


const char* mqtt_server = "192.168.1.79";

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;


#ifndef STASSID
#define STASSID "Livebox-19E2"
#define STAPSK  "hcTzdCTZAqgVLxmKbM"
#endif

const char* ssid     = STASSID;
const char* password = STAPSK;

struct joystickValues {
  int x1;  int y1; bool btn1;
  int x2;  int y2; bool btn2;
};

const int Joy1x = D7;
const int Joy1y = D6;
const int Joy2x = D5;
const int Joy2y = D0;
const int btn1 = 4;
const int btn2 = 5;

unsigned long lastTime = 0;  
unsigned long timerDelay = 100;  // send readings timer

void blink();
int readAnalogValue(int pin);

// Use WiFiClient class to create TCP connections
WiFiClient espClient;
PubSubClient mqttClient(espClient);

DynamicJsonDocument jsonDoc(1024);
char data[80];




void setup_wifi() {
  blink();
  blink();
  blink();

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
    // We start by connecting to a WiFi network
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.println();
  Serial.println();
  Serial.print("Wait for WiFi... ");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    digitalWrite(LED_BUILTIN, LOW);
    delay(300);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(300);
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup_pins(){
  Serial.println("Init Pins");
  
  pinMode(Joy1x, OUTPUT);
  pinMode(Joy1y, OUTPUT);
  pinMode(Joy2x, OUTPUT);
  pinMode(Joy2y, OUTPUT);
  pinMode(btn1, INPUT);
  pinMode(btn2, INPUT);
  
  digitalWrite(Joy1x, LOW);
  digitalWrite(Joy1y, LOW);
  digitalWrite(Joy2x, LOW);
  digitalWrite(Joy2y, LOW);

}

void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      mqttClient.publish("stat/remote/status", "connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the BUILTIN_LED pin as an output

  Serial.begin(115200);

  delay(1000);

  setup_wifi();

  mqttClient.setServer(mqtt_server, 1883);

  setup_pins();

}

struct joystickValues read_joystick()
{
  struct joystickValues values;

  /////////////////////////////////////////////////////////////////////
  values.x1 = readAnalogValue(Joy1x);
  values.y1 = readAnalogValue(Joy1y);
  values.btn1 = (bool)(digitalRead(btn1) == HIGH); // digitalRead()
  /////////////////////////////////////////////////////////////////////
  values.x2 = readAnalogValue(Joy2x);
  values.y2 = readAnalogValue(Joy2y);
  values.btn2 = (bool)(digitalRead(btn2) == HIGH); // digitalRead()
  /////////////////////////////////////////////////////////////////////

  return values;

}


String mainTopic = "proto1";
String cmndTopic = "cmnd";
String topic = cmndTopic + "/" + mainTopic + "/move";


void loop() {

if (!mqttClient.connected())     reconnect();
  
  mqttClient.loop();

  if ((millis() - lastTime) > timerDelay) {
  // Read value of command
  
  joystickValues values = read_joystick();
  //jsonDoc["x1"] = values.x1;
  //jsonDoc["y1"] = values.y1;
  
  //serializeJson(jsonDoc, payload); 
int bt1 = 0; if (values.btn1) bt1 = 0;
long servoVal1 = map(constrain(values.x1,12,900), 12 , 900, 0, 9)*20;
//long servoVal1 = map(values.x1, 0 , 1023, 0, 18)*10;
  String pload = "\"x1\": " + String(servoVal1) ;
  pload  = pload + ", \"y1\": " + String(values.y1) ;
  pload  = pload + ", \"btn1\": " + bt1;
  pload  = pload + ", \"x2\": " + String(values.x2) ;
  pload  = pload + ", \"y2\": " + String(values.y2) ;
  pload  = pload + ", \"btn2\": 0";
 
 // This sends off your payload. 
  String payload = "{" + pload + "}";
  payload.toCharArray(data, (payload.length() + 1));

    digitalWrite(LED_BUILTIN, HIGH);
    mqttClient.publish(topic.c_str(), data);

  lastTime = millis();
  }

}

// /------------------------------------------------------------------------------/
// /---- SUPPORT METHODES --------------------------------------------------------/
// /------------------------------------------------------------------------------/
void blink() {
  digitalWrite(LED_BUILTIN, LOW);
  delay(200);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(200);
  digitalWrite(LED_BUILTIN, LOW);
}

int readAnalogValue(int pin) {
  digitalWrite(pin, HIGH);
  int a = analogRead(A0);
  //wait(5);
  digitalWrite(pin, LOW);
  return a;
}
