#ifndef client_control
#define client_control

//***************FORMATO PPM*********************
typedef struct {
     unsigned char red,green,blue;
} PPMPixel;

typedef struct {
     int x, y;
     PPMPixel *data;
} PPMImage;

void writePPM(const char *, PPMImage *);
PPMImage *readPPM(const char *);
void negativoPPM(PPMImage *);
double PSNR(PPMImage *,PPMImage *);//calcola il PSNR
double MSE(PPMImage *,PPMImage *);
//************************************************

#endif