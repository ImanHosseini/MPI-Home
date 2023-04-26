#include <mpi.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <cassert>
#include "bitmap_image.hpp"

#define W 400
#define H 400
#define DX 0.01

/*
    indexing:
    0  - 1   - 2   - 3   - 4 - ... - W-1
    W  - W+1 - W+2 - W+3 - ..      - 2W-1
    2W - 2W+1- wW+2-..
    ..
*/

float* phi;
float* nphi;

int proc_cnt = 0;
int world_rank = 0;
char* pname = new char[100];
int rlen;

// convert flat id to xi, yi & vice-versa
void fid2xy(int idx, int& xi, int& yi){
    yi = idx / W;
    xi = idx % W;
}

void xy2fid(int xi, int yi, int& idx){
    idx = yi * W + xi;
}

int xy2fid(int xi, int yi){
    int idx = yi * W + xi;
    if(idx >= H*W){
        printf("%d, %d\n",xi, yi);
    }
    assert(idx < H*W);
    return idx;
}

void save_data(){
    bitmap_image img(W, H);
    for(auto i = 0; i < W; i++){
        for(auto j = 0; j < H; j++){
            unsigned char val = int(std::clamp(phi[xy2fid(i,j)], 0.0f, 1.0f) * 255.0f);
            // int val = 0;
            unsigned char red = val;
            unsigned char blue = val;
            unsigned char green = val;
            img.set_pixel(i, j, rgb_t{red,blue,green});
        }
    }
    img.save_image(std::string("output.bmp"));
}

// imposes dirchlet conditions
void constraint(float* phi){
    for(int d = 0; d < 120; d++){
        int xi = 10 + d;
        int yi = 10 + d;
        int idx = xy2fid(xi, yi);
        phi[idx] = 1.0f;
        idx = xy2fid(xi + 240 - 2*d, yi);
        phi[idx] = 1.0f;
        idx = xy2fid(xi, yi + 240 - 2*d);
        phi[idx] = 1.0f;
        idx = xy2fid(xi + 240 - 2*d, yi + 240 - 2*d);
        phi[idx] = 1.0f;
    }
    // rectangle boundary
    for(int dx = 0; dx < W; dx++){
        int idx = xy2fid(dx, 0);
        phi[idx] = 1.0f;
        idx = xy2fid(dx, H-1);
        phi[idx] = 1.0f;
    }
    for(int dy = 0; dy < H; dy++){
        int idx = xy2fid(0, dy);
        phi[idx] = 1.0f;
        idx = xy2fid(W-1, dy);
        phi[idx] = 1.0f;
    }
}

void inregion(int idx, int nx, int ny, int xoff, int yoff){
    int xi,yi;
    fid2xy(idx,nx,ny);

}

void constraint(float* phi, int nx, int ny, int xoff, int yoff){
    for(int d = 0; d < 120; d++){
        int xi = 10 + d;
        int yi = 10 + d;
        int idx = xy2fid(xi, yi);
        phi[idx] = 1.0f;
        idx = xy2fid(xi + 240 - 2*d, yi);
        phi[idx] = 1.0f;
        idx = xy2fid(xi, yi + 240 - 2*d);
        phi[idx] = 1.0f;
        idx = xy2fid(xi + 240 - 2*d, yi + 240 - 2*d);
        phi[idx] = 1.0f;
    }
    // rectangle boundary
    for(int dx = 0; dx < W; dx++){
        int idx = xy2fid(dx, 0);
        phi[idx] = 1.0f;
        idx = xy2fid(dx, H-1);
        phi[idx] = 1.0f;
    }
    for(int dy = 0; dy < H; dy++){
        int idx = xy2fid(0, dy);
        phi[idx] = 1.0f;
        idx = xy2fid(W-1, dy);
        phi[idx] = 1.0f;
    }
}

void step(const float* phi, int nx, int ny, float * phi_new){
    for(int xi = 1; xi < nx-1; xi++){
        for(int yi = 1; yi < ny-1; yi++){
            int idx = xy2fid(xi, yi);
            int iw =  xy2fid(xi-1, yi);
            int ie =  xy2fid(xi+1, yi);
            int is =  xy2fid(xi, yi+1);
            int in =  xy2fid(xi, yi-1);
            phi_new[idx] = 0.25f*(phi[iw] + phi[ie] + phi[in] + phi[is]);
        }
    }
    // apply boundary condition
    constraint(phi_new);
}

void init_data(float* &phi){
    phi = (float*)calloc(H*W, sizeof(float));
    constraint(phi);
}

int main(int argc, char** argv) {
    init_data(phi);
    nphi = (float*) malloc(sizeof(float)*H*W);
    for(int t = 0; t < 100; t++){
        step(phi, W, H, nphi);
        float* tmp = phi;
        phi = nphi;
        nphi = tmp;
    }
    save_data();
}