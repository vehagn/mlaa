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

void edgeDetect(const int w, const int h, const int *pix, int *edge) {
    const int LIMIT = 64;

    int ind, ind_r, ind_b;

    enum {l_edge=1, r_edge=2, b_edge=4, t_edge=8};

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
                edge[(pix[ind]<pix[ind_b])?(ind):(ind_b)] += (pix[ind]>pix[ind_b])?(t_edge):(b_edge);
            }
        }
    }
}

class Shape {
    public:
        void setStart(int _x, int _y) {start_x = _x; start_y = _y;}
        void setEnd(int _x, int _y) {end_x = _x; end_y = _y;}

        void setType(int _y) {
            if (_y == -32) type = 32;
            else if (start_y < end_y) type = 1;
            else if (start_y > end_y) type = 2;
            else if (end_y == _y)     type = 4;
            else                      type = 8;
        }

        int getStartX(){return start_x;}
        int getStartY(){return start_y;}

        int getEndX(){return end_x;}
        int getEndY(){return end_y;}

        int getType(){return type;}

        float getLength(){
            return sqrt((start_x-end_x)*(start_x-end_x) + (start_y-end_y)*(start_y-end_y));
        }

        void reset(){
            start_x = -1;
            start_y = -1;
            end_x   = -1;
            end_y   = -1;
            type    = -1;
        }

    private:
        int start_x = -1;
        int start_y = -1;
        int end_x = -1;
        int end_y = -1;
        int type = -1; // 1 = \, 2 = /, 4 = U, 8 = ^
};


void findShapes(const int w, const int h, int *edge, std::vector<Shape> &shapes) {
    enum {l_edge=1, r_edge=2, b_edge=4, t_edge=8};

    int v_edge, h_edge;

    int ind,ind_b;
    Shape shape;

    bool found_right;
    bool found_left;
    bool found_hor ;

    int *ort = new int[2];
    //ort[0] = -1; ort[1] = -1;
    //TODO: Create a column sweep based on the row sweep
    for (int j=0; j<h-1; j++) {
        for (int i=0; i<w; i++) {
            ind   = idx(w,h,i  ,j  );
            ind_b = idx(w,h,i  ,j+1);

            // Create slices that are the collective of the upper and lower pixel being checked.
            v_edge = ((edge[ind] | edge[ind_b]) & (l_edge | r_edge));
            h_edge = ((edge[ind] & b_edge) || (edge[ind_b] & t_edge));

            // Reset flags for each line
            if (i==0) {
                found_right = false;
                found_left  = false;
                found_hor   = false;
                ort[0] = -1;
                ort[1] = -1;
                // If we're at the edge we might find L shapes
                if (h_edge) {
                    found_hor = true;
                    shape.setStart(i,j);
                }
            }

            // True if we have a vertical edge and haven't found a left edge yet.
            if (v_edge && !found_left && !found_hor) {
                ort[0] = i; //(v_edge & r_edge);
                ort[1] = (edge[ind] & v_edge)?(j):(j+1);
                found_left = true;
            }

            if (h_edge && !found_hor) {
                shape.setStart(ort[0],ort[1]);
                found_hor = true;
            }

            if (v_edge && found_hor) {
                ort[0] = i;
                ort[1] = (edge[ind] & v_edge)?(j):(j+1);
            }

            // We've run out of horizontal edge and will end the shape
            if (!h_edge && found_hor) {
                if (!found_left){//We've found an L shape
                    shape.setEnd(ort[0],ort[1]);
                    shape.setType(-32);
                }
                else{
                    // If both ort edges are on the same side we have a u-shape and should
                    // decrease the length of our shape
                    // TODO: Fix u shapes! These following shapes give different results:
                    // XXXX     0000
                    // X00X =/= 0XX0
                    ort[0] = (ort[1]==shape.getStartY())?(ort[0]-1):(ort[0]);
                    shape.setEnd(ort[0],ort[1]);
                    shape.setType(j);
                }

                shapes.push_back(shape);

                // Reset flags
                found_right = false;
                found_left  = false;
                found_hor   = false;
                shape.reset();
            }
        }
    }
}

void blend(const int w, const int h, int *pix, std::vector<Shape> &shapes) {
    int c_old, c_opp;
    float b1, b2;
    float A;

    int x_len, y_len;
    int x_start, y_start;
    int x_end, y_end;

    float eps = 1e-8;

    int type;

    Shape shape;

    int *tmp = new int[w*h];
    for (int i=0; i<w*h; i++) {tmp[i] = pix[i];}

    for (int s=0; s<shapes.size(); s++) {
        shape = shapes.at(s);

        x_start = shape.getStartX();
        y_start = shape.getStartY();
        x_end   = shape.getEndX();
        y_end   = shape.getEndY();

        type    = shape.getType();

        x_len = abs(x_start - x_end);
        y_len = abs(y_start - y_end);
        if (type ==    1) {
            for (int i=0; i<x_len; i++) {
                b1 = 0.5 - (i    )/(1.0*x_len);
                b2 = 0.5 - (i+1.0)/(1.0*x_len);
                // If we're in a split cell only calculate the triangle area,
                // else the area will be zero.
                A = 0.5*fabs((fabs(b1+b2)>eps)?(b1+b2):(b1));
                if (b1 > 0) {
                    c_old = tmp[idx(w,h,x_start+i,y_start)];
                    c_opp = tmp[idx(w,h,x_start+i,y_end  )];
                    pix[idx(w,h,x_start+i,y_start)] = (1-A)*c_old + A*c_opp;
                    std::cout << y_start << " " << c_opp << std::endl;
                }
                if (b2 < 0) {
                    c_old = tmp[idx(w,h,x_start+i,y_end  )];
                    c_opp = tmp[idx(w,h,x_start+i,y_start)];
                    pix[idx(w,h,x_start+i,y_end  )] = (1-A)*c_old + A*c_opp;
                }
            }
        }
        if (type ==    2) {
            for (int i=0; i<x_len; i++) {
                b1 = 0.5 - (i    )/(1.0*x_len);
                b2 = 0.5 - (i+1.0)/(1.0*x_len);
                // If we're in a split cell only calculate the triangle area,
                // else the area will be zero.
                A = 0.5*fabs((fabs(b1+b2)>eps)?(b1+b2):(b1));

                if (b1 > 0) {
                    c_old = tmp[idx(w,h,x_start+(i+1),y_start)];
                    c_opp = tmp[idx(w,h,x_start+(i+1),y_end  )];
                    pix[idx(w,h,x_start+(i+1),y_start)] = (1-A)*c_old + A*c_opp;
                }
                if (b2 < 0) {
                    c_old = tmp[idx(w,h,x_start+(i+1),y_end  )];
                    c_opp = tmp[idx(w,h,x_start+(i+1),y_start)];
                    pix[idx(w,h,x_start+(i+1),y_end  )] = (1-A)*c_old + A*c_opp;
                }
            }
        }
        if (type ==    4) {
            for (int i=0; i<x_len; i++) {
                b1 = 0.5 - (i    )/(1.0*x_len);
                b2 = 0.5 - (i+1.0)/(1.0*x_len);

                A = 0.5*fabs((fabs(b1+b2)>eps)?(b1+b2):(b1));

                c_old = tmp[idx(w,h,x_start+(i+1),y_start  )];
                c_opp = tmp[idx(w,h,x_start+(i+1),y_start-1)];

                pix[idx(w,h,x_start+(i+1),y_start)] = (1-A)*c_old + A*c_opp;

            }
        }
        if (type ==    8) {
            for (int i=0; i<x_len; i++) {
                b1 = 0.5 - (i    )/(1.0*x_len);
                b2 = 0.5 - (i+1.0)/(1.0*x_len);

                A = 0.5*fabs((fabs(b1+b2)>eps)?(b1+b2):(b1));

                c_old = tmp[idx(w,h,x_start+(i+1),y_start  )];
                c_opp = tmp[idx(w,h,x_start+(i+1),y_start-1)];

                pix[idx(w,h,x_start+(i+1),y_start)] = (1-A)*c_old + A*c_opp;

            }
        }
        if (type ==   16) {
        // TODO
        }
        if (type ==   32) {
        // TODO
        }
        if (type ==   64) {
        // TODO
        }
        if (type ==  128) {
        // TODO
        }
        if (type ==  256) {
        // TODO
        }
        if (type ==  512) {
        // TODO
        }
        if (type == 1024) {
        // TODO
        }
        if (type == 2048) {
        // TODO
        }
    }
    delete[] tmp;
}

int main (int argc, char *argv[]) {

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
    // Create L shape
    pix[idx(w,h, 0,9)]=255;
    pix[idx(w,h, 1,9)]=255;
    pix[idx(w,h, 2,9)]=255;

    pix[idx(w,h,19,9)]=255;

    // Create U shape
    pix[idx(w,h,16,0)]=255;
    pix[idx(w,h,17,0)]=255;
    pix[idx(w,h,18,0)]=255;

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

    int *edge = new int[w*h];
    for (int i=0; i<w*h; i++) edge[i] = 0;

    edgeDetect(w, h, pix, edge);

    int *edgepix = new int[w*h];
    for (int i=0; i<w; i++) {
        for (int j=0; j<h; j++) {
            ind = idx(w,h,i,j);
            edgepix[ind] = 255;
            if (edge[ind]) edgepix[ind] = edge[ind];
        }
    }
    writeImg(edgepix,w,h,"edge.bmp");

    std::vector<Shape> shapes;
    findShapes(w, h, edge, shapes);

    int *shapepix = new int[w*h];
    for (int i=0; i<w*h; i++) {shapepix[i] = 255;}

    Shape shape;
    for (int i=0; i<shapes.size(); i++){
        shape = shapes.at(i);
        std::cout << std::setw(2) << shape.getStartX() << " " << std::setw(2) << shape.getStartY() << " -> ";
        std::cout << std::setw(2) << shape.getEndX()   << " " << std::setw(2) << shape.getEndY()   << " \t ";
        std::cout << std::setw(2) << shape.getType()    << std::endl;
        shapepix[idx(w,h,shape.getStartX(), shape.getStartY())] = 0;
        shapepix[idx(w,h,shape.getEndX()  , shape.getEndY()  )] = 0;
    }
    writeImg(shapepix, w, h, "shapes.bmp");

    std::cout << std::endl << "   ";
    for (int i=0; i<w; i++) std::cout << std::setw(2) << i << " ";
    for (int j=0; j<h; j++) {
        std::cout << std::endl << std::setw(2) << j << " ";
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

    blend(w,h,pix,shapes);

    writeImg(pix,w,h,"mlaa.bmp");

    std::cout << std::endl << "   ";
    for (int i=0; i<w; i++) std::cout << std::setw(4) << i;
    for (int j=0; j<h; j++) {
        std::cout << std::endl << std::setw(3) << j << " ";
        for (int i=0; i<w; i++) {
            std::cout << std::setw(3) << pix[idx(w,h,i,j)] << " ";
        }
    }
    std::cout << std::endl;

    return 0;
}
