#include <stdio.h>
#include <iostream>
#include <stdlib.h>

void writeImg(int *pix, int w, int h) {

    int filesize = 54 + 3*w*h;
    unsigned char *img = new unsigned char[3*w*h];
    int ind = 0;

    for (int i=0; i<w; i++) {
        for (int j=0; j<h; j++) {
            ind = i + ((h-1)-j)*w;
            img[ind*3+2] = (unsigned char)(pix[ind]); // r
            img[ind*3+1] = (unsigned char)(pix[ind]); // g
            img[ind*3+0] = (unsigned char)(pix[ind]); // b
        }
    }

    // Magic header mumbo-jumbo
    unsigned char bmpfileheader[14] = {'B','M',0,0,0,0,0,0,0,0,54,0,0,0};
    unsigned char bmpinfoheader[40] = {40,0,0,0,0,0,0,0,0,0,0,0,1,0,24,0};
    unsigned char bmppad[3] = {0,0,0};

    bmpfileheader[ 2] = (unsigned char)(filesize    );
    bmpfileheader[ 3] = (unsigned char)(filesize>> 8);
    bmpfileheader[ 4] = (unsigned char)(filesize>>16);
    bmpfileheader[ 5] = (unsigned char)(filesize>>24);

    bmpinfoheader[ 4] = (unsigned char)(       w    );
    bmpinfoheader[ 5] = (unsigned char)(       w>> 8);
    bmpinfoheader[ 6] = (unsigned char)(       w>>16);
    bmpinfoheader[ 7] = (unsigned char)(       w>>24);
    bmpinfoheader[ 8] = (unsigned char)(       h    );
    bmpinfoheader[ 9] = (unsigned char)(       h>> 8);
    bmpinfoheader[10] = (unsigned char)(       h>>16);
    bmpinfoheader[11] = (unsigned char)(       h>>24);

    FILE *f;

    f = fopen("img.bmp","wb");
    fwrite(bmpfileheader,1,14,f);
    fwrite(bmpinfoheader,1,40,f);
    for (int i=0; i<h; i++) {
        fwrite(img+(w*(h-i-1)*3),3,w,f);
        fwrite(bmppad,1,(4-(w*3)%4)%4,f);
    }

    free(img);
    fclose(f);
}

int main (int argc, char *argv[]) {

    int w = 500;
    int h = 300;
    int ind = 0;

    int *pix = new int[w*h];

    // Create an image with some features
    for (int i=0; i<w; i++) {
        for (int j=0; j<h; j++) {
            ind = i + ((h-1)-j)*w;
            pix[ind] = 255; // Make sure we fill a color;
            if (    i       < w/4) pix[ind] = 127;
            if (j           < h/4) pix[ind] =  63;
            if (j  +i - h/4 < h  ) pix[ind] =  31;
            if (j-2*i + 4*h < w  ) pix[ind] =   0;
        }
    }

    writeImg(pix,w,h);

    return 0;
}
