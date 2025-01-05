#include "MQTTManager.h"

#include <WiFi.h>
AsyncMqttClient mqtt;
Ticker mqttTim;
Ticker msgTim;
String mqttCommand;

//#define LED_PIN 2


static const char* _broker;
static uint16_t _port;
static const char* _clientId;
uint32_t counter = 0;

void setupMqtt(const char* broker, uint16_t port, const char* clientId) {
  _broker = broker;
  _port = port;
  _clientId = clientId;

  mqtt.setClientId(_clientId);
  mqtt.setServer(_broker, _port);
  mqtt.onConnect(Mqtt_onConnect);
  mqtt.onDisconnect(Mqtt_onDisconnect);
  mqtt.onMessage(Mqtt_onMessage);
  setMqttTim(true);
}

void Mqtt_onConnect(bool sessionPresent) {
  Serial.println("Mqtt connected");
  setMqttTim(false);
  msgTim.attach_ms(5000, publishMessage);
  mqtt.subscribe("device/led", 0);
}

void Mqtt_onDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Mqtt disconnected!");
  setMqttTim(true);
}

void Mqtt_onMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
    char buffer[len + 1];  // Create a buffer with space for null terminator
    memcpy(buffer, payload, len);  // Copy payload to buffer
    buffer[len] = '\0';  // Null-terminate the buffer
    
    mqttCommand = String(buffer);  // Convert to String
    mqttCommand.trim();   // Remove leading/trailing spaces
  
    Serial.println("LED Command Received: " + mqttCommand);
    
    /*if (mqttCommand == "turn on") {
        digitalWrite(LED_PIN, HIGH);
        Serial.println("LED Turned ON");
    } 
    else if (mqttCommand == "turn off") {
        digitalWrite(LED_PIN, LOW);
        Serial.println("LED Turned OFF");
    } 
    else {
        Serial.println("Invalid Command Received!");
    } */
}


void tryConnectMqtt() {
  if (WiFi.isConnected() && !mqtt.connected()) {
    mqtt.connect();
  }
}

void setMqttTim(bool enabled) {
  if (enabled) {
    mqttTim.attach_ms(5000, tryConnectMqtt);
  } else {
    mqttTim.detach();
  }
}

void publishMessage() {
  if (mqtt.connected()) {
    char buff[64];
    sprintf(buff, "Cnt: %u", ++counter);
    mqtt.publish("device/msg", 0, false, buff);
  }
}
