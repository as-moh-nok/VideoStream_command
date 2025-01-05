#include <BluetoothSerial.h>
#include "SerialBLT.h"

extern BluetoothSerial SerialBT;  // Access global SerialBT

void BLTbegin(const char* deviceName) {
    Serial.begin(115200);
    SerialBT.begin(deviceName);
    Serial.println("Bluetooth initialized. Ready for pairing...");
}

String BLTreadCommand() {
    if (SerialBT.available()) {
        String command = SerialBT.readStringUntil('\n');
        command.trim();
        Serial.println("Received: " + command);
        return command;
    }
    return "";
}
