#include "Recorder.h"
#include "esp_camera.h"
#include "Arduino.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"



/* private define */
#define Recorder_initFrameSize(W, H)                {(uint16_t) W, (uint16_t) H}
/* private typedef */
typedef struct {
    camera_fb_t*    Frames[RECORDER_QUEUE_SIZE];
    uint8_t         InIndex;
    uint8_t         OutIndex;
    uint8_t         IsRunning;
} Recorder_Params;
typedef struct {
    uint16_t        Width;
    uint16_t        Height;
} Recorder_FrameSize;
/* private variables */
extern const Recorder_FrameSize framesizes[FRAMESIZE_INVALID];
TaskHandle_t recorderTaskHandle;
Recorder_Params recorder = {0};


void Recorder_task(void* pvParams) {
    long nextCapture = 0;
    camera_fb_t* fb;
    for(;;) {
        if (recorder.IsRunning) {
            if (nextCapture <= millis()) {
                // set next frame time
                nextCapture += RECORDER_FRAME_INTERVAL;
                
                // check buffer have size
                fb = esp_camera_fb_get();
                Recorder_addFrame(fb);
            }
        }
        delay(1);
    }
}

void Recorder_init(void) {
    camera_config_t config;
    
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = CAM_Y2_GPIO_NUM;
    config.pin_d1 = CAM_Y3_GPIO_NUM;
    config.pin_d2 = CAM_Y4_GPIO_NUM;
    config.pin_d3 = CAM_Y5_GPIO_NUM;
    config.pin_d4 = CAM_Y6_GPIO_NUM;
    config.pin_d5 = CAM_Y7_GPIO_NUM;
    config.pin_d6 = CAM_Y8_GPIO_NUM;
    config.pin_d7 = CAM_Y9_GPIO_NUM;
    config.pin_xclk = CAM_XCLK_GPIO_NUM;
    config.pin_pclk = CAM_PCLK_GPIO_NUM;
    config.pin_vsync = CAM_VSYNC_GPIO_NUM;
    config.pin_href = CAM_HREF_GPIO_NUM;
    config.pin_sccb_sda = CAM_SIOD_GPIO_NUM;
    config.pin_sccb_scl = CAM_SIOC_GPIO_NUM; //updated
//config.pin_sscb_scl = CAM_SIOC_GPIO_NUM;
    config.pin_pwdn = CAM_PWDN_GPIO_NUM;
    config.pin_reset = CAM_RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.jpeg_quality = RECORDER_JPEG_QUALITY;
    config.fb_count = RECORDER_QUEUE_SIZE;
    config.frame_size = RECORDER_FRAME_SIZE;

    
    if (esp_camera_init(&config) != ESP_OK) {
      return;
    }
    
    xTaskCreatePinnedToCore(
        Recorder_task,
        "RecorderTask",
        8192,
        NULL,
        1,
        &recorderTaskHandle,
        0
    );

}

void Recorder_start(void) {
    if (!Recorder_isQueueEmpty()) {
        camera_fb_t* fb;
        while ((fb = Recorder_getFrame())) {
            Recorder_releaseFrame(fb);
        }
    }
    recorder.InIndex = 0;
    recorder.OutIndex = 0;
    recorder.IsRunning = 1;
}
void Recorder_stop(void) {
    recorder.IsRunning = 0;
}

uint8_t Recorder_isRunning(void) {
    return recorder.IsRunning;
}

void Recorder_addFrame(camera_fb_t* fb) {
    if (!Recorder_isQueueFull()) {
        recorder.InIndex++;
        recorder.InIndex = recorder.InIndex % RECORDER_QUEUE_SIZE;
        recorder.Frames[recorder.InIndex] = fb;
    }
    else {
      Recorder_releaseFrame(fb);
    }
}
camera_fb_t* Recorder_getFrame(void) {
    if (Recorder_isQueueEmpty()) {
        return NULL;
    }
    else {
        camera_fb_t* fb = recorder.Frames[recorder.OutIndex];
        recorder.OutIndex++;
        recorder.OutIndex = recorder.OutIndex % RECORDER_QUEUE_SIZE;
        return fb;
    }
}

void Recorder_releaseFrame(camera_fb_t* fb) {
    esp_camera_fb_return(fb);
}

uint8_t Recorder_isQueueFull(void) {
    return ((recorder.InIndex + recorder.OutIndex - RECORDER_QUEUE_SIZE) % RECORDER_QUEUE_SIZE + 1) == RECORDER_QUEUE_SIZE;
}
uint8_t Recorder_isQueueEmpty(void) {
    return recorder.InIndex == recorder.OutIndex;
}

uint16_t Recorder_getWidth(void) {
    return framesizes[RECORDER_FRAME_SIZE].Width;
}
uint16_t Recorder_getHeight(void) {
    return framesizes[RECORDER_FRAME_SIZE].Height;
}


const Recorder_FrameSize framesizes[FRAMESIZE_INVALID] = {
    Recorder_initFrameSize(160, 120),
    Recorder_initFrameSize(128, 160),
    Recorder_initFrameSize(176, 144),
    Recorder_initFrameSize(240, 176),
    Recorder_initFrameSize(320, 240),
    Recorder_initFrameSize(400, 296),
    Recorder_initFrameSize(640, 480),
    Recorder_initFrameSize(800, 600),
    Recorder_initFrameSize(1024, 768),
    Recorder_initFrameSize(1280, 1024),
    Recorder_initFrameSize(1600, 1200),
    Recorder_initFrameSize(2048, 1536),
};
