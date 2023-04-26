#include <mpi.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <cassert>
#include <stdint.h>
#include "bitmap_image.hpp"

#define W_DEFAULT 400
#define H_DEFAULT 400
#define K_DEFAULT 200
#define DX 0.01

uint32_t W = W_DEFAULT;
uint32_t H = H_DEFAULT;
uint32_t K = K_DEFAULT;

uint32_t w = W;
uint32_t h = H;

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
int rank = 0;
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
    // assert(idx < H*W);
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
void Uconstraint(float* phi){
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

void set1(int xi, int yi, float* phi){
    int idx = xy2fid(xi, yi);
    if(rank == 1){
            if(yi < K) return;
            idx -= W*K;
        }else{
            if(yi >= K) return;
        }
    phi[idx] = 1.0f;
}

void constraint(float* phi){
    for(int d = 0; d < 120; d++){
        int xi = 10 + d;
        int yi = 10 + d;
        set1(xi, yi , phi);
        set1(xi + 240 - 2*d, yi, phi);
        set1(xi, yi + 240 - 2*d, phi);
        set1(xi + 240 - 2*d, yi + 240 - 2*d, phi);
    }
    // rectangle boundary
    for(int dx = 0; dx < W; dx++){
        if(rank == 0){
            int idx = xy2fid(dx, 0);
            phi[idx] = 1.0f;
        }else{
            int idx = xy2fid(dx, H-1);
            idx -= K*W;
            phi[idx] = 1.0f;
        }
    }
    for(int dy = 0; dy < H; dy++){
        if(dy < K){
            int idx = xy2fid(0, dy);
            phi[idx] = 1.0f;
            idx = xy2fid(W-1, dy);
            phi[idx] = 1.0f;
        }else{
            int idx = xy2fid(0, dy);
            idx -= K*W;
            phi[idx] = 1.0f;
            idx = xy2fid(W-1, dy);
            idx -= K*W;
            phi[idx] = 1.0f;
        }
    }
}


void step(const float* phi, int nx, int ny, float * phi_new){
    int xs = 1;
    int ys = 1;
    int xe = nx - 1;
    int ye = ny - 1;
    if(rank == 0){
        ye++;
    }
    for(int xi = xs; xi < xe; xi++){
        for(int yi = ys; yi < ye; yi++){
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
    Uconstraint(phi);
}

template <typename T = uint32_t> void getV(const char* name, T& val){
    char* r = std::getenv(name); 
    if(r){
        val = static_cast<T>(std::stoll(r));
    }
}

void halosync(){
    MPI_Status status;
    if(rank == 0){
        MPI_Send(phi + K*W, W, MPI_FLOAT, 1, 0, MPI_COMM_WORLD);
        MPI_Recv(phi + K*W + W, W, MPI_FLOAT, 1, 0, MPI_COMM_WORLD, &status);
    }
    if(rank == 1){
        MPI_Recv(phi, W, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Send(phi + W, W, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
    }
}

int main(int argc, char** argv) {
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_cnt);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Get_processor_name(pname, &rlen);
    printf("[%s] : rank %d out of %d processors\n", pname, rank, proc_cnt);
    // get values from env
    // note: all env vars OMPI_* will be exported to all ranks
    getV("OMPI_W", W);
    getV("OMPI_H", H);
    getV("OMPI_K", K);
    printf("[%s]: W=%d, H=%d, K=%d\n",pname, W, H, K);    
    assert(proc_cnt == 2);
    if(rank == 0){
        h = K + 1;
        init_data(phi);
        nphi = (float*)calloc(H*W, sizeof(float));
    }
    if(rank == 1){
        h = H - K;
        phi = (float*)calloc((H-K)*W, sizeof(float));
        constraint(phi);
        nphi = (float*)calloc((H-K)*W, sizeof(float));
    }
    MPI_Status status;
    if(rank == 0){
        MPI_Send(phi + K*W, W*(H-K), MPI_FLOAT, 1, 0, MPI_COMM_WORLD);
    }else{
        MPI_Recv(phi, W*(H-K), MPI_FLOAT, 0, 0, MPI_COMM_WORLD, &status);
    }
    for(int t = 0; t < 200; t++){
        step(phi, w, h, nphi);
        float* tmp = phi;
        phi = nphi;
        nphi = tmp;
        halosync();
    }
    if(rank == 1){
        MPI_Send(phi, W*(H-K), MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
    }else{
        MPI_Recv(phi + K*W, W*(H-K), MPI_FLOAT, 1, 0, MPI_COMM_WORLD, &status);
        save_data();
    }    
    MPI_Finalize();
}