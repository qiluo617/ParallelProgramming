#include <iostream>
#include <mpi.h>
#include <unistd.h>
#include <ctime>
#define MCW MPI_COMM_WORLD

using namespace std;

int main(int argc, char **argv){
    
    int rank, size;
    int timer;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);
    srand(time(NULL)+rank);
    
    if(rank == 0){
        timer = rand()%10+1;
        cout<<"The bomb is gonna explode in "<< timer<<" secends."<<endl;
        int dest = rand()%size;
        cout<<"Passing from "<<rank<<" to "<<dest<<"."<<endl;
        MPI_Send(&timer,1,MPI_INT,dest,0,MCW);
    }
    
    
    while(1){
        MPI_Recv(&timer,1,MPI_INT,MPI_ANY_SOURCE,0,MCW,MPI_STATUS_IGNORE);
        
        //Time's up. Done.
        if(timer==-1)break;
        
        sleep(1);
        
        //About to explodes.
        if(--timer == 0){
            cout<<"BOMB! Current holder is "<< rank<<"."<<endl;
            
            int bomb=-1;
            for(int i=0;i<size;++i){
                MPI_Send(&bomb,1,MPI_INT,i,0,MCW);
            }
            
        }
        else{
            int dest = rand()%size;
            cout<<"Passing from "<<rank<<" to "<<dest<<"."<<endl;
            MPI_Send(&timer,1,MPI_INT,dest,0,MCW);
        }
    }
    
    
    MPI_Finalize();
    
    return 0;
}


