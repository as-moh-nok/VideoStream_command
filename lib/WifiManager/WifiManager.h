#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <WiFi.h>
#include <Ticker.h>


void setupWifi(const char* ssid, const char* password);
void Wifi_onEvent(arduino_event_t *event);
void tryConnectWifi();
void setWifiTim(bool enabled);

extern Ticker wifiTim;

#endif
