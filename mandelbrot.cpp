#include <iostream>
#include <fstream>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <algorithm>
#include <math.h>
#include <string>
#include <mpi.h>
#define MCW MPI_COMM_WORLD

using namespace std;

const int imageSize = 512;
const int maxIter = 1000;

struct Complex {
    double r;
    double i;
};

Complex operator + (Complex s, Complex t){
    Complex v;
    v.r = s.r + t.r;
    v.i = s.i + t.i;
    return v;
}

Complex operator * (Complex s, Complex t){
    Complex v;
    v.r = s.r*t.r - s.i*t.i;
    v.i = s.r*t.i + s.i*t.r;
    return v;
}

int main(int argc, char **argv){
    int rank, size;
    int hdr;
    unsigned char line[3*imageSize];
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);
    FILE *fp = fopen("mandel.ppm", "wb");
    MPI_Barrier(MCW);
    
    time_t start, end;
    
    if(rank==0){
        hdr = fprintf(fp, "P6\n%d %d\n255\n", imageSize, imageSize);
        time(&start);
    }

    MPI_Bcast(&hdr,1,MPI_INT,0,MCW);
    
    int start_id = (rank*imageSize)/size;
    int end_id = ((rank+1)*imageSize) /size;
    
     for (int k = start_id; k < end_id; k++){
        for (int l = 0; l < imageSize; l++){
            int iter = 0;
            Complex z,c;
            c.r= l * (2.5/(double)imageSize)-1.8;
            c.i= k * (2.5/(double)imageSize)-1.2;
            z.r = c.r;
            z.i = c.i * (z.r-c.r);
            z.r = c.i*c.r;

            while(iter < maxIter && z.i*z.i+z.r*z.r < 4){
                z = z*z + c;
                iter++;
            }
            line[3*l] = iter % 250 ;  /* red */
            line[3*l+1] = iter % 150 ;  /* green */
            line[3*l+2] = (iter * iter) % 165 ;  /* blue */

        }
         fseek(fp,hdr+3*imageSize*k,SEEK_SET);
         fwrite(line, 1, 3*imageSize, fp);
    }
    time(&end);
    double time_taken = double(end - start);
    cout << "Time taken by " << rank << " is : " << time_taken ;
    cout << " sec."<< endl;
    
    MPI_Barrier(MCW);
    MPI_Finalize();
    

    return 0;
    
}
