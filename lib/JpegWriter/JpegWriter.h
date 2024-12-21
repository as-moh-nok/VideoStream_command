#ifndef _JPEG_WRITER_H_
#define _JPEG_WRITER_H_

#include "esp_camera.h"

void JpegWriter_save(const char* filename, camera_fb_t* fb);

#endif /* _JPEG_WRITER_H_ */
