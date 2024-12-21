#include "AviWriter.h"
#include "Arduino.h"

/* private defines */
#define AVI_HEADER_LEN                  240

/* private variables */
extern const uint8_t aviDC[4];
extern const uint8_t avi1[4];
extern const uint8_t aviIdx1[4];
extern const uint8_t aviZero[4];
extern const uint8_t aviHeader[AVI_HEADER_LEN];


void AviWriter_create(Avi* avi, const char* filename, uint16_t width, uint16_t height) {
    avi->AviFile = fopen(filename, "w");
    avi->IdxFile = fopen("/sdcard/idx.tmp", "w");
    if (avi->AviFile == NULL || avi->IdxFile == NULL) {
        return;
    }
    // write header
    AviWriter_writeHeader(avi);
    // write frame size
    AviWriter_writeFrameSize(avi, width, height);
    // seek to lend of header
    fseek(avi->AviFile, AVI_HEADER_LEN, SEEK_SET);
    
    avi->IdxOffset = 4;
}
void AviWriter_writeHeader(Avi* avi) {
    fwrite(aviHeader, 1, AVI_HEADER_LEN, avi->AviFile);
}   
void AviWriter_addFrame(Avi* avi, camera_fb_t* fb) {
    uint32_t remains;
    uint32_t pos;

    avi->JpegSize = fb->len;
    // update movi size
    avi->MoviSize += fb->len;
    // write dc
    fwrite(aviDC, 1, 4, avi->AviFile);
    // write zero
    fwrite(aviZero, 1, 4, avi->AviFile);
    // write frame
    fwrite(fb->buf, 1, fb->len, avi->AviFile);
    // check align jpeg
    remains = (4 - (avi->JpegSize & 0x00000003)) & 0x00000003;
    // update idx file
    AviWriter_writeUInt32_BE(avi->IdxFile, avi->IdxOffset);
    AviWriter_writeUInt32_BE(avi->IdxFile, avi->JpegSize);

    avi->IdxOffset = avi->IdxOffset + fb->len + remains + 8;

    avi->JpegSize += remains;
    avi->MoviSize += remains;

    if (remains > 0) {
        fwrite(aviZero, 1, remains, avi->AviFile);
    }

    pos = ftell(avi->AviFile);
    fseek(avi->AviFile, pos - avi->JpegSize - 4, SEEK_SET);
    AviWriter_writeUInt32_BE(avi->AviFile, avi->JpegSize);

    pos = ftell(avi->AviFile);
    fseek(avi->AviFile, pos + 6, SEEK_SET);
    fwrite(avi1, 1, 4, avi->AviFile);

    pos = ftell(avi->AviFile);
    fseek(avi->AviFile, pos + avi->JpegSize - 10 , SEEK_SET);
}
void AviWriter_close(Avi* avi, uint64_t duration) {
    uint32_t endOfFile;

    endOfFile = ftell(avi->AviFile);

    fseek(avi->AviFile, 4 , SEEK_SET);
    AviWriter_writeUInt32_BE(
        avi->AviFile,
        avi->MoviSize + 240 + 16 * avi->FramesCount + 8 * avi->FramesCount
    );


    float fRealFPS = (1000.0f * (float) avi->FramesCount) / ((float)duration);
    float fmicroseconds_per_frame = 1000000.0f / fRealFPS;
    uint8_t iAttainedFPS = round(fRealFPS);
    uint32_t us_per_frame = round(fmicroseconds_per_frame);
    fseek(avi->AviFile, 0x20 , SEEK_SET);
    AviWriter_writeUInt32_BE(avi->AviFile, us_per_frame);

    unsigned long max_bytes_per_sec = avi->MoviSize * iAttainedFPS / avi->FramesCount;

    fseek(avi->AviFile, 0x24 , SEEK_SET);
    AviWriter_writeUInt32_BE(avi->AviFile, max_bytes_per_sec);

    fseek(avi->AviFile, 0x30 , SEEK_SET);
    AviWriter_writeUInt32_BE(avi->AviFile, avi->FramesCount);

    fseek(avi->AviFile, 0x8c , SEEK_SET);
    AviWriter_writeUInt32_BE(avi->AviFile, avi->FramesCount);

    fseek(avi->AviFile, 0x84 , SEEK_SET);
    AviWriter_writeUInt32_BE(avi->AviFile, (int)iAttainedFPS);

    fseek(avi->AviFile, 0xe8 , SEEK_SET);
    AviWriter_writeUInt32_BE(avi->AviFile, avi->MoviSize + avi->FramesCount * 8 + 4);

    fseek(avi->AviFile, endOfFile, SEEK_SET);

    fclose(avi->IdxFile);

    fwrite(aviIdx1, 1, 4, avi->IdxFile);

    AviWriter_writeUInt32_BE(avi->AviFile, avi->FramesCount * 16);

    avi->IdxFile = fopen("/sdcard/idx.tmp", "r");

    if (avi->IdxFile != NULL)  {

    }  else  {
        return;
    }

    char AteBytes[8];

    for (int i = 0; i < avi->FramesCount; i++) {
        fread (AteBytes, 1, 8, avi->IdxFile);
        fwrite(aviDC, 1, 4, avi->AviFile);
        fwrite(aviZero, 1, 4, avi->AviFile);
        fwrite(AteBytes, 1, 8, avi->AviFile);
    }

    fclose(avi->AviFile);
    fclose(avi->IdxFile);
}
void AviWriter_writeFrameSize(Avi* avi, uint16_t width, uint16_t height) {
    uint8_t buffer[2];

    buffer[0] = (uint8_t) width;
    buffer[1] = (uint8_t)(width >> 8);
    fseek(avi->AviFile, 0x40, SEEK_SET);
    fwrite(buffer, 1, 2, avi->AviFile);
    fseek(avi->AviFile, 0xA8, SEEK_SET);
    fwrite(buffer, 1, 2, avi->AviFile);

    buffer[0] = (uint8_t) height;
    buffer[1] = (uint8_t) (height >> 8);
    fseek(avi->AviFile, 0x44, SEEK_SET);
    fwrite(buffer, 1, 2, avi->AviFile);
    fseek(avi->AviFile, 0xAC, SEEK_SET);
    fwrite(buffer, 1, 2, avi->AviFile);
}

void AviWriter_writeUInt32_BE(FILE* file, uint32_t value) {
    uint8_t buffer[4];
    buffer[0] = (uint8_t) value;
    value >>= 8;
    buffer[1] = (uint8_t) value;
    value >>= 8;
    buffer[2] = (uint8_t) value;
    value >>= 8;
    buffer[3] = (uint8_t) value;

    fwrite(buffer, 1, 4, file);
}

const uint8_t aviZero[4] = {
    0x00, 0x00, 0x00, 0x00,
};
const uint8_t aviDC[4] = {
    0x30, 0x30, 0x64, 0x63,
};
const uint8_t aviIdx1[4] {
    0x41, 0x56, 0x49, 0x31,
};
const uint8_t avi1[4] = {
    0x69, 0x64, 0x78, 0x31,
};

const uint8_t aviHeader [AVI_HEADER_LEN] = {
    0x52, 0x49, 0x46, 0x46, 0xD8, 0x01, 0x0E, 0x00, 0x41, 0x56, 0x49, 0x20, 0x4C, 0x49, 0x53, 0x54,
    0xD0, 0x00, 0x00, 0x00, 0x68, 0x64, 0x72, 0x6C, 0x61, 0x76, 0x69, 0x68, 0x38, 0x00, 0x00, 0x00,
    0xA0, 0x86, 0x01, 0x00, 0x80, 0x66, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
    0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x80, 0x02, 0x00, 0x00, 0xe0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4C, 0x49, 0x53, 0x54, 0x84, 0x00, 0x00, 0x00,
    0x73, 0x74, 0x72, 0x6C, 0x73, 0x74, 0x72, 0x68, 0x30, 0x00, 0x00, 0x00, 0x76, 0x69, 0x64, 0x73,
    0x4D, 0x4A, 0x50, 0x47, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x73, 0x74, 0x72, 0x66,
    0x28, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x80, 0x02, 0x00, 0x00, 0xe0, 0x01, 0x00, 0x00,
    0x01, 0x00, 0x18, 0x00, 0x4D, 0x4A, 0x50, 0x47, 0x00, 0x84, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x49, 0x4E, 0x46, 0x4F,
    0x10, 0x00, 0x00, 0x00, 0x6A, 0x61, 0x6D, 0x65, 0x73, 0x7A, 0x61, 0x68, 0x61, 0x72, 0x79, 0x20,
    0x76, 0x33, 0x39, 0x20, 0x4C, 0x49, 0x53, 0x54, 0x00, 0x01, 0x0E, 0x00, 0x6D, 0x6F, 0x76, 0x69,
};
