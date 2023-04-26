#include <mpi.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <cassert>

int proc_cnt = 0;
int world_rank = 0;
char* pname = new char[100];
int rlen;

void greeting(){
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_cnt);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Get_processor_name(pname, &rlen);
    printf("[%s] : rank %d"
           " out of %d processors\n",
           pname, world_rank, proc_cnt);
}

int main(int argc, char** argv) {
    greeting();
    MPI_Finalize();
}