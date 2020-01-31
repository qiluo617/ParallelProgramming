#include <iostream>
#include <mpi.h>
#include <unistd.h>
#include <stdlib.h>
#include <algorithm>
#include <vector>
#include <math.h>
#include <sstream>
#define MCW MPI_COMM_WORLD

using namespace std;

using namespace std;

void display(int *array, int size) {
    for(int i = 0; i<size; i++)
        cout << array[i] << " ";
    cout << endl;
}

int max(int a, int b){
    if(a>b){
        return a;
    }else{
        return b;
    }
}

int min(int a, int b){
    if(b<a){
        return b;
    }else{
        return a;
    }
}

int main(int argc, char **argv){
    int rank, size;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);
    srand(rank);
    
    int unsorted[size];
    int sorted[size];

    if(rank == 0){
        cout<<"Generate a random list: ";
        for(int i=0; i<size;i++){
            unsorted[i] = rand()%(2*size);
        }
        
        display(unsorted,size);
        
        for(int dest=0; dest<size; dest++){
            MPI_Send(&unsorted[dest],1,MPI_INT,dest,0,MCW);
        }
    }
    
    MPI_Barrier(MCW);
    
    int data;
    int recv_data;
    MPI_Recv(&recv_data,1,MPI_INT,0,0,MCW,MPI_STATUS_IGNORE);

    int step = 0;
    while(step<log2(size)){
        step++;
        int mask = 1 << step;
        int order = (rank & mask) >> step;
        
        while(mask) {
            mask >>=1;
            int dest = rank ^ mask;
            MPI_Send(&recv_data,1,MPI_INT,dest,0,MCW);
            MPI_Recv(&data,1,MPI_INT,dest,0,MCW,MPI_STATUS_IGNORE);
            
            if(order == 0) {  //ascending
                if((rank & mask) == 0) {
                    recv_data = min(data,recv_data); //keep smaller
                } else {
                    recv_data = max(data,recv_data); //keep larger
                }
            }
            if(order == 1){ //descending
                if((rank & mask) == 0) {
                    recv_data = max(data,recv_data); //keep larger
                } else {
                    recv_data = min(data,recv_data);  //keep smaller
                }
            }
            
        }

        MPI_Gather(&recv_data,1,MPI_INT,sorted,1,MPI_INT,0,MCW);
        if(!rank){
            cout<<"After Step " + to_string(step) + ": ";
            display(sorted,size);
        }

    }

    MPI_Finalize();
    return 0;
}

