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

#include <EEPROM.h>

#define EEPROM_SIZE     1

const uint8_t LedPin = 33;
const uint32_t MaxFramesPerFile = 200;
const uint32_t WarmUpFrames = 30; 
char fileName[64];

typedef enum {
    RecordState_CreateFile,
    RecordState_Recording,
    RecordState_CloseFile,  
    Waiting,
} RecordState;

Avi avi;
TaskHandle_t saveTaskHandle;
RecordState recordState;
uint64_t elapsedTime;
String command;

char tempStr[40];

static esp_err_t init_sdcard(void);
void saveFrameTask(void* pvParams);

void setup() {
    int videoNum;
    Serial.begin(115200);
    
    pinMode(LedPin, OUTPUT);

    // SD camera init
    esp_err_t card_err = init_sdcard();
    if (card_err != ESP_OK) {
        Serial.printf("SD Card init failed with error 0x%x", card_err);
        digitalWrite(LedPin, HIGH);
        return;
    }

    recordState = Waiting;

    EEPROM.begin(EEPROM_SIZE);
    videoNum = EEPROM.read(0);
    sprintf(fileName, "/sdcard/video_%02d.avi", videoNum); 
    
    Recorder_init();

    xTaskCreatePinnedToCore(
        saveFrameTask,      // Task Function
        "SaveFrameTask",    // Task Name
        8192,               // Task Stack Size
        NULL,               // Task Args
        1,                  // Task Priority
        &saveTaskHandle,    // Task Handle Object
        1                   // Core Number (0, 1)
    );

}

void loop() {
    if(Serial.available()) {
        command = Serial.readStringUntil('\n'); //read serial input

        if (command == "start-record") {
            recordState = RecordState_CreateFile;
            Serial.println("Recording started");
        } else if (command == "stop-record") {
            recordState = RecordState_CloseFile;
            Serial.println("Recording stopped");
        }
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);  // Yield to other tasks
}

void saveFrameTask(void* pvParams) {
    for (;;) {
        switch (recordState) {
            case RecordState_CreateFile:
                Serial.println("creating file...");
                AviWriter_create(
                    &avi, 
                    fileName,
                    Recorder_getWidth(),
                    Recorder_getHeight()
                );
                Recorder_start();
                elapsedTime = millis();
                recordState = RecordState_Recording;
                break;
            case RecordState_Recording:
                Serial.println("start recording...");
                // warmup sensor
                while (avi.FramesCount < WarmUpFrames) {
                    camera_fb_t* fb = Recorder_getFrame();
                    if (fb) {
                        Recorder_releaseFrame(fb);
                        avi.FramesCount++;
                    }
                }
                avi.FramesCount = 0;
                // record video
                while (avi.FramesCount < MaxFramesPerFile) {
                    camera_fb_t* fb = Recorder_getFrame();
                    if (fb) {
                        AviWriter_addFrame(&avi, fb);
                        //sprintf(tempStr, "/sdcard/frame_%03d.jpg", avi.FramesCount);
                        //JpegWriter_save(tempStr, fb);
                        Recorder_releaseFrame(fb);
                        avi.FramesCount++;
                        digitalWrite(LedPin, !digitalRead(LedPin));
                        if (avi.FramesCount % 50 == 0) {
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
                Serial.println((uint32_t) elapsedTime);
                Serial.print("FPS: ");
                Serial.println(MaxFramesPerFile / elapsedTime);
                Serial.println("Recording Done, closing file");
                AviWriter_close(&avi, elapsedTime);
                EEPROM.write(0, EEPROM.read(0) + 1);
                EEPROM.commit();
                //recordState = (RecordState) -1;        // option1: just one video record
                recordState = Waiting;  //option2: Record anothe video
               
                break;
            case Waiting:
                Serial.println("Waiting for new command...");
            // waiting for new command
            default:
                
                break;
        }
        delay(1);
    }
}

static esp_err_t init_sdcard(void) {
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

  if (ret == ESP_OK) {
    Serial.println("SD card mount successfully!");
  }  else  {
    Serial.printf("Failed to mount SD card VFAT filesystem. Error: %s", esp_err_to_name(ret));
  }
  return ret;
}
