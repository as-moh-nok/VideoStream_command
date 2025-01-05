#ifndef MQTTMANAGER_H
#define MQTTMANAGER_H

#include <AsyncMqttClient.h>
#include <Ticker.h>

void setupMqtt(const char* broker, uint16_t port, const char* clientId);
void Mqtt_onConnect(bool sessionPresent);
void Mqtt_onDisconnect(AsyncMqttClientDisconnectReason reason);
void Mqtt_onMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
void tryConnectMqtt();
void setMqttTim(bool enabled);
void publishMessage();

extern AsyncMqttClient mqtt;
extern Ticker mqttTim;
extern Ticker msgTim;
extern String mqttCommand;

#endif
