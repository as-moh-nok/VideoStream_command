#include "JpegWriter.h"


void JpegWriter_save(const char* filename, camera_fb_t* fb) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        return;
    }
    fwrite(fb->buf, 1, fb->len, file);
    fclose(file);
}
