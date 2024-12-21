#include "SDCard.h"
#include <FS.h>
#include <SD_MMC.h>   //SD card
#include "esp_camera.h"
//#include "camera_pins.h"

bool SD_begin()
{
    if(!SD_MMC.begin())
    {
        Serial.println("SD_MMC Mount Failed");
        return false;
    }
    return true;
}

bool check_isSD()
{
    uint8_t cardType = SD_MMC.cardType();

    if(cardType == CARD_NONE){
        Serial.println("No SD_MMC card attached");
        return false;
    }
    return true;
}

void SD_info()
{
    uint8_t cardType = SD_MMC.cardType();

    if(cardType == CARD_NONE){
        Serial.println("No SD_MMC card attached");
        return ;
    }
    
    Serial.print("SD_MMC Card Type: ");
    if(cardType == CARD_MMC){
        Serial.println("MMC");
    } else if(cardType == CARD_SD){
        Serial.println("SDSC");
    } else if(cardType == CARD_SDHC){
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }

    Serial.printf("Total Bytes: %llu\n", SD_MMC.totalBytes());
    Serial.printf("Used Bytes: %llu\n", SD_MMC.usedBytes());
}

void writeImageFile(const char FILE_NAME[], camera_fb_t* fb)
{
    File file = SD_MMC.open(FILE_NAME, FILE_WRITE);
    if( file){
        if(file.write(fb->buf, fb->len))
            Serial.println("Image Write");
         else {
            Serial.println("Image Write Failed");
        }

    file.flush();
    file.close();
    } else
    Serial.println("File doesn't found!");
}


File readImageFile(const char FILE_NAME[])
{
    File imagefile = SD_MMC.open(FILE_NAME, FILE_READ);
    if(!imagefile)
    {
        Serial.println("Failed to open file");
    }
    return imagefile;
}
