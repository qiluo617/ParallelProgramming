#include <iostream>
#include <fstream>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <algorithm>
#include <math.h>
#include <string>
#include <mpi.h>
#include <vector>
#define MCW MPI_COMM_WORLD

using namespace std;

const int world_size = 50; //should be 1024
const float prob = 0.2;
const int days = 101;

const bool random_initial = true;

int getValue(int id, vector<int> world){
    if((id < world_size*world_size) && (id >= 0)){
        return world[id];
    }else{
        return 0;
    }
}

int updateCell(vector<int> world, int current_id){
    int count = 0;
    // x-1, y-1
    count += getValue(current_id - world_size - 1, world);
    //x-1, y
    count += getValue(current_id - world_size, world);
    //x-1, y+1
    count += getValue(current_id - world_size + 1, world);
    //x, y-1
    count += getValue(current_id - 1, world);
    //x, y+1
    count += getValue(current_id + 1, world);
    //x+1, y-1
    count += getValue(current_id + world_size - 1, world);
    //x+1, y
    count += getValue(current_id + world_size, world);
    //x+1, y+1
    count += getValue(current_id + world_size + 1, world);
    
    //update
    int value = world[current_id];
    if((count == 2 || count ==3) && value == 1){
         return 1;
    }else if(count > 3 && value == 1){
        return 0;
    }else if(count < 1 && value == 1){
        return 0;
    }else if(count ==3 && value == 0){
        return 1;
    }else{
        return 0;
    }
    
}

vector<int> combine(int rank, int size, vector<int> world, vector<int> part){
    int offset = world_size/size;
    if(rank==0){
        vector<int> recv_data(world_size*world_size, 0);
        MPI_Gather(&part[0],world_size*offset,MPI_INT,&recv_data.front(),world_size*offset,MPI_INT,0,MCW);
        for(int i = 1; i < size; i++){
            MPI_Send(&recv_data[0],world_size*world_size,MPI_INT,i,0,MCW);
        }
        return recv_data;
    } else{
        MPI_Gather(&part[0],world_size*offset,MPI_INT,NULL,world_size*offset,MPI_INT,0,MCW);
        MPI_Recv(&world[0],world_size*world_size,MPI_INT,0,0,MCW,MPI_STATUS_IGNORE);
    }
    
    return world;
}


int main(int argc, char** argv) {
    
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);
    srand(time(NULL));
    
    //  set up world
    vector<int> world(world_size*world_size, 0);
    
    if(rank == 0){
        
        if(random_initial){
            for(int i=0; i < world_size*world_size; i++){
                
                float p = (rand()%100)/100.0;
                
                if(p < prob){
                    world[i] = 1;
                }
                
            }
        }else{ // creating a 'glider gun' at left top edge
            world[1] = 1;
            world[world_size + 2] = 1;
            world[2*world_size] = 1;
            world[2*world_size +1] = 1;
            world[2*world_size +2] = 1;
        }
        
        for(int i=1; i<size; i++){
            MPI_Send(&world[0],world_size*world_size,MPI_INT,i,0,MCW);
        }
        
    }else{
        MPI_Recv(&world[0],world_size*world_size,MPI_INT,0,0,MCW,MPI_STATUS_IGNORE);
    }
    
    float total_time;
    
    for(int i=0; i<days; i++){
        
        double t1,t2;
        t1 = MPI_Wtime();
        
        if(rank == 0){
            cout<< "Day:" << i <<endl;
            if((i+10)%10 == 0){
                //save world
                string filename = "day_" +to_string(i) + ".ppm";
                ofstream outfile(filename, ofstream::app);
                outfile << "P1" <<"\n"<< world_size << " " << world_size <<endl;
                
                for(int i = 0; i < world_size; i++){
                    for(int j = 0; j < world_size; j++){
                        outfile << world[i*world_size+j] << " ";
                    }
                    outfile << "\n";
                }
                outfile.close();
                
            }
        } //seperate work to other processors
            
        int duty = world_size*world_size/size;
        int start_id = rank * duty;
        
        vector<int> part_world(duty*world_size, 0);
        
        for(int i = 0; i < duty; i++){
            part_world[i] = updateCell(world, i+start_id);
        }
        //combine array
        world = combine(rank, size, world, part_world);
        
        t2 = MPI_Wtime();
        if(!rank){
            total_time += t2 - t1;
        }
        
    }
    
   if(!rank)cout <<"Total run time is "<< total_time <<" seconds for "<<size <<" processors."<<endl;
    
    MPI_Finalize();
    return 0;
}
