#ifndef _CAPTURE_PHOTO_H
#define _CAPTURE_PHOTO_H

#include "esp_camera.h"
#include <driver/rtc_io.h>


void camera_config(camera_config_t &config);
bool camera_init(camera_config_t &config);
void warmup_sensor(int counter);
camera_fb_t* capture_photo(void);


#endif //_CAPTURE_PHOTO_H