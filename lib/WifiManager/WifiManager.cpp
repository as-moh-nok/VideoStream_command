#include "wifiManager.h"

Ticker wifiTim;
static const char* _ssid;
static const char* _password;

void setupWifi(const char* ssid, const char* password) {
  _ssid = ssid;
  _password = password;
  WiFi.onEvent(Wifi_onEvent);
  WiFi.setAutoReconnect(true);
  WiFi.begin(_ssid, _password);
  setWifiTim(true);
}

void Wifi_onEvent(arduino_event_t *event) {
  switch (event->event_id) {
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.println("Wifi Disconnected!");
      setWifiTim(true);
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.println("Wifi Connected");
      setWifiTim(false);
      break;
    default:
      break;
  }
}

void tryConnectWifi() {
  if (!WiFi.isConnected()) {
    WiFi.begin(_ssid, _password);
  }
}

void setWifiTim(bool enabled) {
  if (enabled) {
    wifiTim.attach_ms(10000, tryConnectWifi);
  } else {
    wifiTim.detach();
  }
}
