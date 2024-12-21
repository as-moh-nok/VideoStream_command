#ifndef _SDCARD_H
#define _SDCARD_H

#include <FS.h>
#include <SD_MMC.h>   //SD card

bool SD_begin();
bool check_isSD();
void SD_info();
void writeImageFile(const char FILE_NAME[], camera_fb_t* fb);
File readImageFile(const char FILE_NAME[]);

#endif //_SDCARD_H