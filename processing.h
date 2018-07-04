#ifndef PROCESSING_H
#define PROCESSING_H

int detect_stopper_color(int r, int g, int b);
void die(const char *errstr);
char* russian_ocr(PIX* img, int index);
void _process_frame_tickertext(AVFrame *frame, int width, int height, int index);
void rgb2lab(float R, float G, float B,float *L, float *a, float *b);
void late_fusion(PIX* img, int index);
void writetofile(char* text, int index);

#endif
