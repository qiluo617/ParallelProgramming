#include <iostream>
#include <mpi.h>
#include <stdlib.h>
#define MCW MPI_COMM_WORLD

using namespace std;

int Global_sum(int number, int rank, int size, MPI_Comm mcw) {
    int        sum = number;
    int        data;
    int        dest;
    unsigned   bitmask = 1;
    
    while (bitmask < size) {
        dest = rank ^ bitmask;
        MPI_Sendrecv(&sum, 1, MPI_INT, dest, 0,
                     &data, 1, MPI_INT, dest, 0,
                     mcw, MPI_STATUS_IGNORE);
        sum += data;
        bitmask <<= 1;
    }

    if(rank==0) cout<<"Sum for bitmask is "<<sum <<endl;
    return sum;
}

int main(int argc, char **argv){
    
    int rank, size;
    int data;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);
    srand(rank);
    
    double t1,t2;
    
    // Ring Pass
    t1 = MPI_Wtime();

    int number = rand()%(2*size);
    
    if(rank==0){
        MPI_Send(&number,1,MPI_INT,rank+1,0,MCW);

        MPI_Recv(&data,1,MPI_INT,size-1,0,MCW,MPI_STATUS_IGNORE);
        cout<<"Sum for ring-pass is "<< data<<endl;
    }else{

        int source = (rank-1)%size;
        int dest = (rank+1)%size;
        
        MPI_Recv(&data,1,MPI_INT,source,0,MCW,MPI_STATUS_IGNORE);
        
        data = number + data;

        MPI_Send(&data,1,MPI_INT,dest,0,MCW);

    }

    t2 = MPI_Wtime();
    
    if(!rank)cout <<"run time is "<< t2-t1 <<" for ring-pass"<<endl;
    
    // Tree Pass
    t1 = MPI_Wtime();
    int remain = size, sum_tree = number, half;
    
    while(remain != 1) {
        half = remain/2;
        if(rank < half) {
            MPI_Recv(&data, 1, MPI_INT, rank+half, 0, MCW, MPI_STATUS_IGNORE);
            sum_tree+=data;
        }
        else {
            MPI_Send(&sum_tree, 1, MPI_INT, rank-half, 0, MCW);
        }
        remain = half;
    }
    
    if(rank == 0) cout<<"sum for tree-pass is "<< sum_tree<<endl;
    
    t2 = MPI_Wtime();
    
    if(!rank)cout <<"run time is "<< t2-t1 <<" for tree-pass"<<endl;
    
    // Bitmask Sum
    t1 = MPI_Wtime();
    
    int sum_bitmask = Global_sum(number, rank, size, MCW);
    
    t2 = MPI_Wtime();
    
    if(!rank)cout <<"run time is "<< t2-t1 <<" for bitmask"<<endl;
    
    MPI_Finalize();
    
    return 0;
}
