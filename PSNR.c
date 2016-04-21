#include <stdio.h>
#include <math.h>
#include "image.h"

int main(int argc, char const *argv[]){
	//calcolo del PSNR delle due immaginiP
	PPMImage * image1;
	PPMImage * image2;
	double psnr;

	image1 = readPPM(argv[1]);
	image2 = readPPM(argv[2]);
	
	psnr = PSNR(image1, image2);

	printf("%f\n",psnr);
	return 0;
}