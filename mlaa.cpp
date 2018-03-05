#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <vector>

inline int idx(int w, int h, int i, int j) {
    return i + ((h-1)-j)*w;
}

void writeImg(const int *red, const int *blue, const int *green, const int w, const int h, const char* fname) {
    // Create and save a .bmp image file using red, blue, green channels
    int filesize = 54 + 3*w*h;
    unsigned char *img = new unsigned char[3*w*h];
    int ind = 0;

    for (int i=0; i<w; i++) {
        for (int j=0; j<h; j++) {
            ind = idx(w,h,i,j);
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
            ind   = idx(w,h,i  ,j  );
            ind_r = idx(w,h,i+1,j  );
            if (abs(pix[ind] - pix[ind_r]) > LIMIT) {
                // Pick the pixel with the highest value
                ver[(pix[ind]>pix[ind_r])?(ind):(ind_r)] = 1;
            }
        }
    }

    // Find horizontal edges. Scan vertical.
    for (int i=0; i<w; i++) {
        for (int j=0; j<h-1; j++) {
            ind   = idx(w,h,i  ,j  );
            ind_b = idx(w,h,i  ,j+1);
            if (abs(pix[ind] - pix[ind_b]) > LIMIT) {
                // Pick the pixel with the highest value
                hor[(pix[ind]>pix[ind_b])?(ind):(ind_b)] = 1;
            }
        }
    }
}

struct Shape {
    int start_x = -1;
    int start_y = -1;
    int stop_x  = -1;
    int stop_y  = -1;
//    int turns =  0; // max two turns
    int type  = -1;
//    int dir   = -1; // last direction [n,e,s,w] = [0,1,2,3];
};

void checkNbd(const int w, const int h, int i, int j, int *edge, int *checked, Shape shape){
    int nbd;
    int k[8] = { 0, 1, 1, 1, 0,-1,-1,-1};
    int l[8] = { 1, 1, 0,-1,-1,-1, 0, 1};

    //  7 0 1
    //  6 X 2
    //  5 4 3

    for (int n=0; n<8; n++) {
        nbd = idx(w,h,i+k[n],j+l[n]);
        if ((nbd < w*h) && (nbd > 0)) {
            if (checked[nbd]) {continue;}
            checked[nbd] = true;
            if (edge[nbd]) {
                shape.stop_x = i+k[n];
                shape.stop_y = j+l[n];
                checkNbd(w,h,i+k[n],j+l[n],edge,checked,shape);
                return;
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
            ind = idx(w,h,i,j);
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
    writeImg(ver,w,h,"0ver.bmp");
    writeImg(hor,w,h,"0hor.bmp");

    int *edge = new int[w*h];

    for (int i=0; i<w; i++) {
        for (int j=0; j<h; j++) {
            ind = idx(w,h,i,j);
            edge[ind] = 255;
            if ((hor[ind] == 1) || (ver[ind] == 1)) {
                edge[ind] = 1;
            }
        }
    }
    writeImg(edge,w,h,"edge.bmp");


    std::vector<Shape> shapes;

    int *checked = new int[w*h];

    for (int i=0; i<w*h; i++) checked[i] = 0;


    int ix, jy;
    for (int i=0; i<w; i++) {
        ix = i;
        for (int j=0; j<h; j++) {
            jy = j;
            ind = idx(w,h,i,j);
            if (edge[ind] && !checked[ind]) {
                checked[ind] = true;
                shapes.push_back(Shape());
                shapes.back().start_x = i;
                shapes.back().start_y = j;
                // Check neigbourhood
                checkNbd(w,h,i,j,edge,checked,shapes.back());
            }
        }
    }
    std::cout << shapes.size() << std::endl;

    return 0;
}
