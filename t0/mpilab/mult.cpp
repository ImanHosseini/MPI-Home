#include <mpi.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <cassert>
#include <stdint.h>
#include <cstdlib>

#define N_DEFAULT 900
#define M_DEFAULT 450

uint32_t n = N_DEFAULT;
uint32_t m = M_DEFAULT;

int proc_cnt = 0;
int rank = 0;
char* pname = new char[100];
int rlen;

int *a, *b, *c;

void getV(const char* name, uint32_t& val){
    char* r = std::getenv(name);
    if(r){
        val = static_cast<uint32_t>(std::stoll(r));
    }
}

void init_data(){
    if(rank == 0){
            for(uint32_t i = 0; i < n*n; i++){
        a[i] = (std::rand() % 61) - 30;
        b[i] = (std::rand() % 61) - 30;
    }
    }
}

void send_to_s1(){
    if(rank == 0){
        MPI_Send(a, n*n, MPI_INT, 1, 0, MPI_COMM_WORLD);
        MPI_Send(b, n*n, MPI_INT, 1, 0, MPI_COMM_WORLD);
    }else{
        MPI_Status status;
        MPI_Recv(a, n*n, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(b, n*n, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    }
}

void compute(){
    int ystart = 0;
    int yend = n;
    if(rank == 0){
        yend = m;
    }else{
        ystart = m;
    }
    for(uint32_t xi = 0; xi < n; xi++){
        for(uint32_t yi = ystart; yi < yend; yi++){
            int sum = 0;
            for(int k = 0; k < n; k++){
                sum += a[xi + k*n]*b[k + yi*n];
            }
            c[xi + n*yi] = sum;
        }
    }
}

void collect(){
    if(rank == 0){
        MPI_Send(c + m*n, (n-m)*n, MPI_INT, 1, 0, MPI_COMM_WORLD);
    }else{
        MPI_Status status;
        MPI_Recv(c + m*n, (n-m)*n, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    }
}

int main(int argc, char** argv){
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_cnt);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Get_processor_name(pname, &rlen);
    assert(proc_cnt == 2);
    getV("OMPI_N", n);
    getV("OMPI_M", m);
    if(rank == 0){
        printf("n=%u, m=%u, split ratio: %.2f\n", n, m, ((float)m/(float)n));
    }
    a = (int*) malloc(sizeof(int) * n * n);
    b = (int*) malloc(sizeof(int) * n * n);
    c = (int*) malloc(sizeof(int) * n * n);
    double t0 = MPI_Wtime();
    init_data();
    send_to_s1();
    compute();
    collect();
    double t1 = MPI_Wtime();
    if(rank == 0){
        printf("Finished in %.2f (ms)\n", (t1 - t0)*1000000.0);
    }
    MPI_Finalize();
}
