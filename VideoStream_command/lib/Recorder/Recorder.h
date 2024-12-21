#ifndef _RECORDER_H_
#define _RECORDER_H_

#include "esp_camera.h"

#define RECORDER_FRAME_INTERVAL             50 //ms
#define RECORDER_QUEUE_SIZE                 60 
#define RECORDER_FRAME_SIZE                 FRAMESIZE_QVGA
#define RECORDER_PIXEL_FROMAT               PIXFORMAT_JPEG
#define RECORDER_JPEG_QUALITY               10

/**
 * @brief Camera pinout
 */
#define CAM_PWDN_GPIO_NUM     32
#define CAM_RESET_GPIO_NUM    -1
#define CAM_XCLK_GPIO_NUM      0
#define CAM_SIOD_GPIO_NUM     26
#define CAM_SIOC_GPIO_NUM     27
#define CAM_Y9_GPIO_NUM       35
#define CAM_Y8_GPIO_NUM       34
#define CAM_Y7_GPIO_NUM       39
#define CAM_Y6_GPIO_NUM       36
#define CAM_Y5_GPIO_NUM       21
#define CAM_Y4_GPIO_NUM       19
#define CAM_Y3_GPIO_NUM       18
#define CAM_Y2_GPIO_NUM        5
#define CAM_VSYNC_GPIO_NUM    25
#define CAM_HREF_GPIO_NUM     23
#define CAM_PCLK_GPIO_NUM     22



void Recorder_init(void);
void Recorder_start(void);
void Recorder_stop(void);

uint32_t Recorder_getFramesCount(void);

uint8_t Recorder_isRunning(void);

void Recorder_addFrame(camera_fb_t* fb);
camera_fb_t* Recorder_getFrame(void);
void Recorder_releaseFrame(camera_fb_t* fb);

uint8_t Recorder_isQueueFull(void);
uint8_t Recorder_isQueueEmpty(void);

uint16_t Recorder_getWidth(void);
uint16_t Recorder_getHeight(void);

#endif /* _RECORDER_H_ */
