#ifndef BLUETOOTH_HANDLER_H
#define BLUETOOTH_HANDLER_H

#include <BluetoothSerial.h>


void BLTbegin(const char* deviceName = "ESP32-Serial");
String BLTreadCommand();

#endif
