#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <math.h>

void writeImg(const int *red, const int *blue, const int *green, const int w, const int h, const char* fname) {
    // Create and save a .bmp image file using red, blue, green channels
    int filesize = 54 + 3*w*h;
    unsigned char *img = new unsigned char[3*w*h];
    int ind = 0;

    for (int i=0; i<w; i++) {
        for (int j=0; j<h; j++) {
            ind = i + ((h-1)-j)*w;
            img[ind*3+2] = (unsigned char)(red[ind]  ); // r
            img[ind*3+1] = (unsigned char)(blue[ind] ); // g
            img[ind*3+0] = (unsigned char)(green[ind]); // b
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

    f = fopen(fname,"wb");
    fwrite(bmpfileheader,1,14,f);
    fwrite(bmpinfoheader,1,40,f);
    for (int i=0; i<h; i++) {
        fwrite(img+(w*(h-i-1)*3),3,w,f);
        fwrite(bmppad,1,(4-(w*3)%4)%4,f);
    }

    free(img);
    fclose(f);
}

void writeImg(const int *grey, const int w, const int h, const char* fname) {
    // Create and save a .bmp file in greyscale calling the rgb-version.
    writeImg(grey, grey, grey, w, h, fname);
}

void edgeDetect(const int *pix, int *ver, int *hor, int w, int h) {
    const int LIMIT = 64;

    int ind, ind_r, ind_b;

    // Find vertical edges. Scan horizontal.
    for (int i=0; i<w-1; i++) {
        for (int j=0; j<h; j++) {
            ind   = i   + ((h-1)-(j  ))*w;
            ind_r = i+1 + ((h-1)-(j  ))*w;
            ind_b = i   + ((h-1)-(j+1))*w;
            if (abs(pix[ind] - pix[ind_r]) > LIMIT) {
                // Pick the pixel with the highest value
                ver[(pix[ind]>pix[ind_r])?(ind):(ind_r)] = 0;
            }
        }
    }

    // Find horizontal edges. Scan vertical.
    for (int i=0; i<w; i++) {
        for (int j=0; j<h-1; j++) {
            ind   = i   + ((h-1)-(j  ))*w;
            ind_r = i+1 + ((h-1)-(j  ))*w;
            ind_b = i   + ((h-1)-(j+1))*w;
            if (abs(pix[ind] - pix[ind_b]) > LIMIT) {
                // Pick the pixel with the highest value
                hor[(pix[ind]>pix[ind_b])?(ind):(ind_b)] = 0;
            }
        }
    }
}

int main (int argc, char *argv[]) {

    int w =  200;
    int h =  100;
    int ind = 0;

    int *fill = new int[w*h];

    for (int i=0; i<w*h; i++) fill[i] = 255;

    int *pix = new int[w*h];

    // Create an image with some features
    for (int i=0; i<w; i++) {
        for (int j=0; j<h; j++) {
            ind = i + ((h-1)-j)*w;
            pix[ind] = 255; // Make sure we fill a color;
            if (   i         < w/4) pix[ind] = 127;
            if (     j       < h/4) pix[ind] =  63;
            if (   i+j - h/4 < h  ) pix[ind] =  31;
            if (-2*i+j + 4*h < w  ) pix[ind] =   0;
        }
    }

    writeImg(pix,w,h,"raw.bmp");

    int *ver = new int[w*h];
    int *hor = new int[w*h];

    for (int i=0; i<w*h; i++) {
        ver[i] = 255;
        hor[i] = 255;
    }

    edgeDetect(pix, ver, hor, w, h);

    writeImg(pix,ver,hor,w,h,"edge.bmp");

    return 0;
}
