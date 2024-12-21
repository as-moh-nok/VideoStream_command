#ifndef _AVI_WRITER_H_
#define _AVI_WRITER_H_

#include "esp_camera.h"

typedef struct {
    FILE*           AviFile;
    FILE*           IdxFile;
    uint32_t        FramesCount;
    uint32_t        MoviSize;
    // Temp
    uint32_t        IdxOffset;
    uint32_t        JpegSize;
    
} Avi;

void AviWriter_create(Avi* avi, const char* filename, uint16_t width, uint16_t height);
void AviWriter_addFrame(Avi* avi, camera_fb_t* fb);
void AviWriter_close(Avi* avi, uint64_t duration);


void AviWriter_writeHeader(Avi* avi);
void AviWriter_writeFrameSize(Avi* avi, uint16_t width, uint16_t height);
void AviWriter_writeUInt32_BE(FILE* file, uint32_t value);

#endif /* _AVI_WRITER_H_ */
