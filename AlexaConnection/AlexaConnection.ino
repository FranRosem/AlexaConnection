#include "project.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>
#include <DHT.h>

/* Wifi Properties */
WiFiClientSecure wifiClient = WiFiClientSecure();

/* Broker MQTT Properties */
PubSubClient mqttClient(wifiClient);
const char* OUT_TOPIC_HUMIDITY = ""; // your topic humidity"
const char* OUT_TOPIC_TEMPERATURE = ""; // your topic temperature"
const char* OUT_TOPIC_SERVOMOTOR = ""; // your topic servomotor"

/* MCU ESP32 Properties */
const char* CLIENT_ID = ""; // unique client id
const char* OUT_SHADOW_HDT11 = ""; // aws shadow update;

/* DHT Temperature-Humidity Sensor */
const int DHT_PIN = 23;
const int DHT_TYPE = 11;
DHT dht(DHT_PIN, DHT_TYPE);

/* Servo Motor */
Servo servomotor;
const int SERVO_PIN = 12;

/* Json Format */
StaticJsonDocument<JSON_OBJECT_SIZE(1)> outputDoc;
StaticJsonDocument<JSON_OBJECT_SIZE(1)> inputDoc;
char outputBuffer[128];

/* Functions */
void turnOnServo() {
  servomotor.write(60);
}

void turnOffServo() {
  servomotor.write(180);
}

// PubSubClient callback function
void callback(const char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += String((char) payload[i]);
  }
  servodegrees(topic, payload, message);
}

void servodegrees(const char* topic, byte* payload, String message){
  if (String(topic) == OUT_TOPIC_SERVOMOTOR) {
    Serial.println("Message from topic " + String(topic) + ": " + message);
    DeserializationError err = deserializeJson(inputDoc, payload);
    if (!err) {
      String action = String(inputDoc["action"].as<char*>());
      if (action == "ON") {
        turnOnServo();
      }
      else if (action == "OFF") {
        turnOffServo();
      }
    }
  }
}

void connectToWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Couldn't connect to WiFi.");
    while(1) delay(100);
  }
  Serial.println("Connected to \"" + String(WIFI_SSID) + "\"");
  wifiClient.setCACert(AMAZON_ROOT_CA1);
  wifiClient.setCertificate(CERTIFICATE);
  wifiClient.setPrivateKey(PRIVATE_KEY);
}

void connectToMQTT() {
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(callback);
}

void subscribeToTopic(const char* topic) {
  mqttClient.subscribe(topic);
  Serial.println("Subscribed to \"" + String(topic) + "\"");
}

boolean mqttClientConnect() {
  Serial.println("Connecting to MQTT broker...");
  if (mqttClient.connect(CLIENT_ID)) {
    Serial.println("Connected to " + String(MQTT_BROKER));
    subscribeToTopic(OUT_TOPIC_TEMPERATURE);
    subscribeToTopic(OUT_TOPIC_HUMIDITY);
    subscribeToTopic(OUT_TOPIC_SERVOMOTOR);
    subscribeToTopic(OUT_SHADOW_HDT11);
  }
  else {
    Serial.println("Couldn't connect to MQTT broker.");
  }
  return mqttClient.connected();
}

void publishMessage(const char* topic, String message) {
  mqttClient.publish(topic, message.c_str());
}

void publishMessageJson(const char* topic, float temperature, float humidity, float segs) {
  String message = "{\"state\":{\"desired\":{\"temperature\":" + String(temperature) + "," + "\"humidity\":" + String(humidity) + "," + "\"time\":" + String(segs) + "}}}";
  Serial.println("Temperatura: " + String(temperature) + "\n" + "Humedad: " + String(humidity) + "\n" + "Time: " + String(segs) + "\n");
  
  publishMessage(topic, message);
}

void connectMyThings() {
  dht.begin();
  servomotor.attach(SERVO_PIN);
}

void readDataFromThings() {
  float temperature = dht.readTemperature(); // Gets the values of the temperature
  float humidity = dht.readHumidity(); // Gets the values of the humidity
  float segs = millis(); // Gets the time from Arduino
  publishMessageJson(OUT_SHADOW_HDT11, temperature, humidity, segs);
}

void setup() {
  Serial.begin(115200);
  connectMyThings();
  connectToWiFi();
  connectToMQTT();
}

void loop() {
  if (!mqttClient.connected()) {
    bool connected = mqttClientConnect();     
    delay(1000);
  }
  else {
    mqttClient.loop();
    delay(1000);
    readDataFromThings();
  }
}
