/*lib : async-mqtt-client-0.9.0, pubsub*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Recorder.h"
#include "AviWriter.h"
#include "JpegWriter.h"
// MicroSD
#include "driver/sdmmc_host.h"
#include "driver/sdmmc_defs.h"
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"
// capture photos
#include <CapturePhoto.h>
#include <SerialBLT.h>
#include "MQTTManager.h"
#include "WifiManager.h"

#include <EEPROM.h>

#define EEPROM_SIZE 1

const uint8_t LedPin = 33;
const uint32_t MaxFramesPerFile = 200;
const uint32_t WarmUpFrames = 30;
char fileName[64];

const char WIFI_SSID[]      = "hwa";
const char WIFI_PASS[]      = "wifi1373";

const char MQTT_BROKER[]    = "broker.emqx.io";
const uint16_t MQTT_PORT    = 1883;
const char MQTT_CLIENT_ID[] = "My-IoT-Device";

typedef enum
{
    RecordState_CreateFile,
    RecordState_Recording,
    RecordState_CloseFile,
    RecordState_Waiting,
    RecordState_Capture,
} RecordState;

Avi avi;
TaskHandle_t saveTaskHandle;
TaskHandle_t wifiTaskHandle;

RecordState recordState;
uint64_t elapsedTime;
String command;
camera_fb_t *fb;
char tempStr[40];

 BluetoothSerial SerialBT;  // Declare globally

static esp_err_t init_sdcard(void);
void saveFrameTask(void *pvParams);
void setupWifiMQTT_Task(void *parameter);

void commandState(String command);

void setup()
{
    int videoNum;
    Serial.begin(115200);

    pinMode(LedPin, OUTPUT);

    // SD camera init
    esp_err_t card_err = init_sdcard();
    if (card_err != ESP_OK)
    {
        Serial.printf("SD Card init failed with error 0x%x", card_err);
        digitalWrite(LedPin, HIGH);
        return;
    }

    recordState =RecordState_Waiting;

    EEPROM.begin(EEPROM_SIZE);
    videoNum = EEPROM.read(0);
    sprintf(fileName, "/sdcard/video_%02d.avi", videoNum);

    Recorder_init();

    BLTbegin(); //  initialize  bluetooth


        // Start WiFi and MQTT in a new task
    xTaskCreatePinnedToCore(
        setupWifiMQTT_Task,
        "WiFiMQTTTask",
        8192,
        NULL,
        1,
        &wifiTaskHandle,
        1  // Core 1 for networking
    );

    
    xTaskCreatePinnedToCore(
        saveFrameTask,   // Task Function
        "SaveFrameTask", // Task Name
        8192,            // Task Stack Size
        NULL,            // Task Args
        1,               // Task Priority
        &saveTaskHandle, // Task Handle Object
        1                // Core Number (0, 1)
    );
}

void loop()
{
    //loopMqtt();  // Handle incoming MQTT messages
   
        // Process incoming MQTT command
    if (mqttCommand != "") {
        Serial.println("Handling MQTT command...");
        commandState(mqttCommand);
        mqttCommand = "";  // Clear the command after handling
    }

    //if(BLTreadCommand() != "")
    if (SerialBT.available()) {
    command = SerialBT.readStringUntil('\n');
    command.trim();
    //Serial.println("Received: " + command);
    commandState(command);
    }

    if (Serial.available())
    {
        command = Serial.readStringUntil('\n'); // read serial input
        commandState(command);
    }
    

    vTaskDelay(10 / portTICK_PERIOD_MS); // Yield to other tasks
}

void setupWifiMQTT_Task(void *parameter)
{
    setupWifi(WIFI_SSID, WIFI_PASS);
    setupMqtt(MQTT_BROKER, MQTT_PORT, MQTT_CLIENT_ID);
}

void saveFrameTask(void *pvParams)
{
    static bool printedWaitingMessage = false;  // Flag to track if "Waiting..." has been printed

    for (;;)
    {
        switch (recordState)
        {
        case RecordState_CreateFile:
            Serial.println("creating file...");
            AviWriter_create(
                &avi,
                fileName,
                Recorder_getWidth(),
                Recorder_getHeight());
            Recorder_start();
            elapsedTime = millis();
            recordState = RecordState_Recording;
            break;
        case RecordState_Recording:
            Serial.println("start recording...");
            // warmup sensor
            while (avi.FramesCount < WarmUpFrames)
            {
                camera_fb_t *fb = Recorder_getFrame();
                if (fb)
                {
                    Recorder_releaseFrame(fb);
                    avi.FramesCount++;
                }
            }
            avi.FramesCount = 0;
            // record video
            while (avi.FramesCount < MaxFramesPerFile)
            {
                if (recordState != RecordState_Recording)
                {
                     recordState = RecordState_CloseFile;  // Stop recording immediately
                     break;
                }

                camera_fb_t *fb = Recorder_getFrame();
                if (fb)
                {
                    AviWriter_addFrame(&avi, fb);
                    // sprintf(tempStr, "/sdcard/frame_%03d.jpg", avi.FramesCount);
                    // JpegWriter_save(tempStr, fb);
                    Recorder_releaseFrame(fb);
                    avi.FramesCount++;
                    digitalWrite(LedPin, !digitalRead(LedPin));
                    if (avi.FramesCount % 50 == 0)
                    {
                        Serial.print("Frames: ");
                        Serial.println(avi.FramesCount);
                    }
                }
            }
            Recorder_stop();
            elapsedTime = millis() - elapsedTime;
            recordState = RecordState_CloseFile;
            break;
        case RecordState_CloseFile:
            Serial.print("Elapsed Time: ");
            Serial.println((uint32_t)elapsedTime);
            Serial.print("FPS: ");
            Serial.println(MaxFramesPerFile / elapsedTime);
            Serial.println("Recording Done, closing file");
            AviWriter_close(&avi, elapsedTime);
            EEPROM.write(0, EEPROM.read(0) + 1);
            EEPROM.commit();
            // recordState = (RecordState) -1;        // option1: just one video record
            recordState =RecordState_Waiting; // option2: Record anothe video

            break;

        case RecordState_Capture:
            Serial.println("Capturing frame...");
            fb = capture_photo();
            if (fb)
            {
                sprintf(tempStr, "/sdcard/frame_%03d.jpg", avi.FramesCount);
                JpegWriter_save(tempStr, fb);
                Recorder_releaseFrame(fb);
                Serial.println("Captured photo saved");
            }
            else
            {
                Serial.println("Failed to capture photo");
            }
            recordState =RecordState_Waiting;
            break;

        case RecordState_Waiting:
            if (!printedWaitingMessage)
            { // Check if the message was printed
                Serial.println("Waiting for new command...");
                printedWaitingMessage = true; // Set flag to true after printing
            }
            vTaskDelay(10 / portTICK_PERIOD_MS);  
            break;

        // default:
        //     break;
        }
        // Reset flag when state changes from Waiting to another state
        if (recordState !=RecordState_Waiting)
        {
            printedWaitingMessage = false;
        }

        delay(1);
    }
}

static esp_err_t init_sdcard(void)
{
    esp_err_t ret = ESP_FAIL;
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 10,
    };
    sdmmc_card_t *card;

    Serial.println("Mounting SD card...");
    ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);

    if (ret == ESP_OK)
    {
        Serial.println("SD card mount successfully!");
    }
    else
    {
        Serial.printf("Failed to mount SD card VFAT filesystem. Error: %s", esp_err_to_name(ret));
    }
    return ret;
}

void commandState(String command)
{
if (command == "start-record")
    {
        recordState = RecordState_CreateFile;
        Serial.println("Recording started");
    }
    else if (command == "stop-record")
    {
        recordState = RecordState_CloseFile;
        Serial.println("Recording stopped");
    }
    else if (command == "capture")
    {
        recordState = RecordState_Capture;
        Serial.println("Capturing frame");
    }
}
