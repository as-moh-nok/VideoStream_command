#include "esp_camera.h"
#include "CapturePhoto.h"
//#include "camera_pins.h"
#include "Arduino.h"
#include <driver/rtc_io.h>

/*void camera_config(camera_config_t &config)
{
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
}*/

bool camera_init(camera_config_t &config)
{
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return false;  // Indicate failure
  }
  return true;  // Success
}

void warmup_sensor(int counter)
{
  camera_fb_t* fb = NULL; 
  int frameCount = 0;

  // Warm-up Sensor
  while (frameCount < counter) {
    fb = esp_camera_fb_get();
    if (fb) {
      frameCount++;
      esp_camera_fb_return(fb);
    } else {
      delay(0); // Release CPU to avoid stalling
    }
  }
}

camera_fb_t* capture_photo()
{
  camera_fb_t* fb = esp_camera_fb_get();  // Get a frame buffer
  if (!fb) {
    Serial.println("Camera capture failed");
    return NULL;  // Return NULL on failure
  }
  return fb;  // Return the frame buffer
}
