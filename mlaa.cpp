#include <stdio.h>
#include <iostream>
#include <iomanip>
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

void edgeDetect(const int w, const int h, const int *pix, int *ver, int *hor) {
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

void edgeDetect2(const int w, const int h, const int *pix, int *edge) {
    const int LIMIT = 64;

    int ind, ind_r, ind_b;

    enum {l_edge=1, r_edge=2, t_edge=4, b_edge=8};

    // Find vertical edges. Scan horizontal.
    for (int i=0; i<w-1; i++) {
        for (int j=0; j<h; j++) {
            ind   = idx(w,h,i  ,j  );
            ind_r = idx(w,h,i+1,j  );
            if (abs(pix[ind] - pix[ind_r]) > LIMIT) {
                // Store in pixel with the lowest (darkest) value
                edge[((pix[ind]<pix[ind_r])?(ind):(ind_r))] += (pix[ind]>pix[ind_r])?(l_edge):(r_edge);
            }
        }
    }

    // Find horizontal edges. Scan vertical.
    for (int i=0; i<w; i++) {
        for (int j=0; j<h-1; j++) {
            ind   = idx(w,h,i  ,j  );
            ind_b = idx(w,h,i  ,j+1);
            if (abs(pix[ind] - pix[ind_b]) > LIMIT) {
                // Store in pixel with the lowest (darkest) value
                edge[(pix[ind]<pix[ind_b])?(ind):(ind_b)] += (pix[ind]>pix[ind_b])?(b_edge):(t_edge);
            }
        }
    }
}

struct line {
    int beg_x;
    int beg_y;
    int end_x;
    int end_y;
};

class Shape2 {
    public:
        int start_x = -1;
        int start_y = -1;
        int end_x = -1;
        int end_y = -1;
        int type = -1; // 1 = \, 2 = /, 4 = U, 8 = ^
    private:
};


void findShapes2(const int w, const int h, int *edge) {

    std::vector<Shape2> shapes;

    enum {l_edge=1, r_edge=2, b_edge=4, t_edge=8};

    //int v_edge = (l_edge|r_edge);
    //int h_edge = (b_edge|t_edge);


    int v_edge, h_edge;

    int ind,ind_b;
    int slice;
    Shape2 shape;

    bool found_left  = false;
    bool found_right = false;
    bool found_hor   = false;

    int *ort = new int[2];
    ort[0] = -1;
    ort[1] = -1;

    for (int j=0; j<h-1; j++) {
        for (int i=0; i<w; i++) {
            ind   = idx(w,h,i  ,j  );
            ind_b = idx(w,h,i  ,j+1);

          //  std::cout << std::setw(2) << edge[ind]   << " ";
          //  std::cout << std::setw(2) << edge[ind_b] << std::endl;

            // Create slices that are the collective of the upper and lower pixel being checked.
            v_edge = ((edge[ind] | edge[ind_b]) & (l_edge|r_edge));
            h_edge = ((edge[ind] & t_edge) || (edge[ind_b] & b_edge));

            // Reset flags for each line
            if (i==0) {
                found_left  = false;
                found_right = false;
                found_hor   = false;
                ort[0] = -1;
                ort[1] = -1;
                // If we're at the edge we might find L shapes
                if (h_edge) {
                    found_hor = true;
                    shape.start_x =  i;
                    shape.start_y =  j;
                    shape.type    = 16;
                }
            }

            // True if we have a vertical edge and haven't found a left edge yet.
            if (v_edge && !found_left) {
                found_left = true;
                // We've found a left orthogonal edge,
                // saving its position
                ort[0] = (edge[ind] & v_edge)?(j):(j+1);
            }

            // We've found a right wall to match our left wall
            if (v_edge && found_hor) {
                ort[1] = (edge[ind] & v_edge)?(j):(j+1);
                // Save the edge if we don't have one.
                if (shape.end_y == -1) {
                    shape.end_y = ort[1];
                }
                // Save right shape if it's opposite the left
                if (ort[0] != ort[1]) {
                    shape.end_y = ort[1];
                }
                found_right = true;
            }

            // If we've found a left edge we can start building our shape if we have a
            // horizontal edge.
            if (h_edge && found_left) {
                // Begin shape if it's the first horisontal edge we find.
                if (!found_hor) {
                    found_hor = true;
                    shape.start_x = i;
                    shape.start_y = ort[0];
                }
                shape.end_x = i;
            }

            // Save shape if the horizontal edge stops.
            if (!h_edge && found_hor) {
                // If we haven't found a right edge we have an L shape.
                if (shape.end_y == -1) {
                    shape.end_y =  j;
                }
                // Figure out which shape we have.
                if (shape.type == -1) {
                    if (ort[0] > ort[1]) {
                         shape.type = 1; // shape type Z
                    }
                    else if (ort[0] < ort[1]) {
                        shape.type = 2; // shape type /
                    }
                    else if (ort[0] == j) {
                        shape.type = 4; // shape type U
                    }
                    else {
                        shape.type = 8; // shape ype ^
                    }
                }
                shapes.push_back(shape);

                // reset right and horizontal edge
                found_right = false;
                found_hor   = false;

                // reset shape
                shape.start_x = -1;
                shape.start_y = -1;
                shape.end_x   = -1;
                shape.end_y   = -1;
                shape.type    = -1;
            }
        }
    }


    int *pix = new int[w*h];

    for (int i=0; i<w*h; i++) {pix[i] = 255;}

    for (int i=0; i<shapes.size(); i++){
        shape = shapes.at(i);
        std::cout << std::setw(2) << shape.start_x << " " << std::setw(2) << shape.start_y << " -> ";
        std::cout << std::setw(2) << shape.end_x   << " " << std::setw(2) << shape.end_y   << " \t ";
        std::cout << std::setw(2) << shape.type    << std::endl;
        pix[idx(w,h,shape.start_x, shape.start_y)] = 0;
        pix[idx(w,h,shape.end_x  , shape.end_y  )] = 0;
    }
    writeImg(pix, w, h, "edge2.bmp");
}

void blend(const int left, const int right, int *pix) {

}

struct Shape {
    int start_x = -1;
    int start_y = -1;
    int stop_x  = -1;
    int stop_y  = -1;
    int turns   =  0; // max two turns
    int type    = -1;
//    int dir   = -1; // last direction [n,e,s,w] = [0,1,2,3];
};

void checkNbd(const int w, const int h, int i, int j, int *edge, int *checked, Shape *shape){
    // Check neighbourhooding pixels if they contain an edge and expand shape if criteria are met
    int nbd;
    int k[8] = { 0, 1, 1, 1, 0,-1,-1,-1};
    int l[8] = { 1, 1, 0,-1,-1,-1, 0, 1};

    //  7 0 1
    //  6 X 2
    //  5 4 3

    int t_norm;

    for (int n=0; n<8; n++) {
        // Stop if we have found our shape
        if (shape->turns > 1) {return;}
        nbd = idx(w,h,i+k[n],j+l[n]);
        if ((nbd < w*h) && (nbd > 0)) {
            // Skip if we're already checked pixel
            if (checked[nbd]) {continue;}
            checked[nbd] = 100;
            if (edge[nbd]) {
                t_norm = abs(k[n]) + abs(l[n]);

                if ((shape->turns > 0) || (t_norm == 2)) checked[nbd] = 0;

                // Continue expanding shape if we can
                if (t_norm < 2) {
                    shape->stop_x = i+k[n];
                    shape->stop_y = j+l[n];
                    checkNbd(w,h,i+k[n],j+l[n],edge,checked,shape);
                }
                if ((t_norm == 2) && shape->turns < 1) {
                    shape->stop_x = i+k[n];
                    shape->stop_y = j+l[n];
                    shape->turns++;
                    checkNbd(w,h,i+k[n],j+l[n],edge,checked,shape);
                }
                return;
            }
        }
    }
}

void findShapes(const int w, const int h, int *edge, std::vector<Shape> &shapes) {
    int ind;

    int *checked = new int[w*h];
    for (int i=0; i<w*h; i++) checked[i] = 0;

    for (int i=0; i<w; i++) {
        for (int j=h-1; j>-1; j--) {
            ind = idx(w,h,i,j);
            if (edge[ind] && !checked[ind]) {
                checked[ind] = 10;
                shapes.push_back(Shape());
                shapes.back().start_x = i;
                shapes.back().start_y = j;
                // Check neigbourhood
                checkNbd(w,h,i,j,edge,checked,&shapes.back());
                // The last shape part should be available for next shape
                // checked[idx(w,h,shapes.back().stop_x,shapes.back().stop_y)] = 0;
            }
        }
    }
}

int main (int argc, char *argv[]) {

    enum {l_edge=1, r_edge=2, b_edge=4, t_edge=8};

//    int test = l_edge + t_edge;
//    std::cout << (test & l_edge) << " " << (test & b_edge) << " " << (test & t_edge) << std::endl;

    int w =   20;
    int h =   10;
    int ind =  0;

    int *fill = new int[w*h];

    for (int i=0; i<w*h; i++) fill[i] = 255;

    int *pix = new int[w*h];

    // Create an image with some features
    for (int i=0; i<w; i++) {
        for (int j=0; j<h; j++) {
            ind = idx(w,h,i,j);
            pix[ind] = 255; // Make sure we fill a color;
            //if (   i         < w/4) pix[ind] = 127;
            if (     j       < h/4) pix[ind] =  63;
            if (   i+j - h/4 < h  ) pix[ind] =  31;
            if (-2*i+j + 4*h < w  ) pix[ind] =   0;
        }
    }

//    pix[idx(w,h,0,9)]=127;
//    pix[idx(w,h,0,8)]=127;
//    pix[idx(w,h,1,7)]=127;
//    pix[idx(w,h,2,6)]=127;
//    pix[idx(w,h,3,5)]=127;
//    pix[idx(w,h,4,5)]=127;
//    pix[idx(w,h,5,4)]=127;
//    pix[idx(w,h,6,3)]=127;
//    pix[idx(w,h,5,2)]=127;
//    pix[idx(w,h,4,1)]=127;
//    pix[idx(w,h,3,1)]=127;
//    pix[idx(w,h,2,0)]=127;
//
//    pix[idx(w,h,6,8)]=127;
//    pix[idx(w,h,7,8)]=127;

    writeImg(pix,w,h,"raw.bmp");

    int *ver = new int[w*h];
    int *hor = new int[w*h];

    for (int i=0; i<w*h; i++) {
        ver[i] = 255;
        hor[i] = 255;
    }

    edgeDetect(w, h, pix, ver, hor);
    writeImg(ver,w,h,"0ver.bmp");
    writeImg(hor,w,h,"0hor.bmp");

    int *edge = new int[w*h];
    for (int i=0; i<w*h; i++) edge[i] = 0;

    edgeDetect2(w, h, pix, edge);

    int *edgepix = new int[w*h];
    for (int i=0; i<w; i++) {
        for (int j=0; j<h; j++) {
            ind = idx(w,h,i,j);
            edgepix[ind] = 255;
            if (edge[ind]) edgepix[ind] = edge[ind];
        }
    }

    writeImg(edgepix,w,h,"edge.bmp");
    findShapes2(w, h, edge);
    for (int j=0; j<h; j++) {
        std::cout << std::endl;
        for (int i=0; i<w; i++) {
            if (edge[idx(w,h,i,j)]) {
                std::cout << std::setw(2) << edge[idx(w,h,i,j)] << " ";
            }
            else {
                std::cout << " . ";
            }
        }
    }
    std::cout << std::endl;
//    std::vector<Shape> shapes;
//
//    findShapes(w, h, edge, shapes);
//
//    std::cout << "Number of shapes: " << shapes.size() << std::endl;

    float len;
    Shape shape;

//    for (int i=0; i<shapes.size(); i++){
//        shape = shapes.at(i);
//        std::cout << shape.start_x << " " << shape.start_y << " -> ";
//        std::cout << shape.stop_x  << " " << shape.stop_y  << " \t ";
//        std::cout << shape.turns   << std::endl;
//
//     //   len = sqrt((shape.start_x-shape.stop_x)*(shape.start_x-shape.stop_x) + (shape.start_x-shape.stop_x)*(shape.start_x-shape.stop_x));
//     //   std::cout << len << std::endl;
//
//        for (int i=shape.start_x; i<shape.stop_x; i++) {
//            for (int j=shape.stop_y; j<shape.start_y; j++) {
//                pix[idx(w,h,i,j)] = 0.5*(pix[idx(w,h,i-1,j)] + pix[idx(w,h,i+1,j)]);
//
//            }
//        }
//
//    }

    writeImg(pix,w,h,"aa.bmp");
//    writeImg(checked,w,h,"checked.bmp");

    return 0;
}
