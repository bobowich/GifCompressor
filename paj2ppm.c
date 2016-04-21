#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "image.h"

typedef struct {
     int x, y , z;//x e y sono le dimensioni della matrice // z dimensione della paletta
     PPMPixel *paletta;
     unsigned char *data;
} PAJImage;

PAJImage *readPAJ(const char *);
PPMImage *paj2ppm(PAJImage *);

int main(int argc, char const *argv[])
{
	PPMImage * result;
	PAJImage * image;
	image = readPAJ(argv[1]);
	result = paj2ppm(image);
	writePPM(argv[2],result);	

/*
	int i;
	for(i = 0; i < result->x * result->y; i++){
		printf("%d %d %d\n",result->data[i].red,result->data[i].green,result->data[i].blue);
	}*/

/*	
	//stampo i risultati
	printf("Paletta:\n");
	int i;
	for(i = 0; i < image->z; i++)	printf("%d %d %d\n",image->paletta[i].red,image->paletta[i].green,image->paletta[i].blue);

	printf("Matrice:\n");
	for(i=0; i< image->x * image->y; i++)printf("%d",image->data[i]);
*/
	return 0;
}

PAJImage *readPAJ(const char * filename){
	int c;
	char buff[16];
    PAJImage *img;
    FILE *fp;
    
    //open PPM file for reading
    fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "Unable to open file '%s'\n", filename);
        exit(1);
        }

    //read image format
    if (!fgets(buff, sizeof(buff), fp)) {
        perror(filename);
        exit(1);
        }

    //check the image format
    if (buff[0] != 'P' || buff[1] != 'A' || buff[2] != 'J') {
         fprintf(stderr, "Invalid image format (must be 'PAJ')\n");
         exit(1);
    }

    //alloc memory form image
    img = (PAJImage *)malloc(sizeof(PAJImage));
    if (!img) {
         fprintf(stderr, "Unable to allocate memory\n");
         exit(1);
    }

    //check for comments
    c = getc(fp);
    while (c == '#') {
    while (getc(fp) != '\n') ;
         c = getc(fp);
    }

    ungetc(c, fp);

    //read image size information
    if (fscanf(fp, "%d %d %d", &img->x, &img->y, &img->z) != 3) {
         fprintf(stderr, "Invalid image size (error1 loading '%s')\n", filename);
         exit(1);
    }

    while (fgetc(fp) != '\n') ;
    
    //memory allocation for  data
    img->data = (unsigned char*)malloc(img->x * img->y);
    img->paletta =(PPMPixel*)malloc(sizeof(PPMPixel)*img->z);

    if (!img) {
         fprintf(stderr, "Unable to allocate memory\n");
         exit(1);
    }

    fread(img->paletta,3,img->z,fp);

    fread(img->data,1,img->x * img->y,fp);

 	return img;
}

PPMImage *paj2ppm(PAJImage * image){
	PPMImage *result;
	result = (PPMImage *)malloc(sizeof(PPMImage));

	result->x = image->x;
	result->y = image->y;

	result->data = (PPMPixel *)malloc(sizeof(PPMPixel)*image->x*image->y);

	//riempio la matrice
	int i,j;
	for(i = 0; i < result->x * result->y; i++){
		j = image->data[i];
		result->data[i].red = image->paletta[j].red;
		result->data[i].green = image->paletta[j].green;
		result->data[i].blue = image->paletta[j].blue;
	}

	return result;
}
