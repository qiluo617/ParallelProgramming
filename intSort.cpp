#include <iostream>
#include <mpi.h>
#include <unistd.h>
#include <stdlib.h>
#include <algorithm>
#include <vector>
#define MCW MPI_COMM_WORLD

using namespace std;
const int len = 20;

void display(int *array, int size) {
    for(int i = 0; i<size; i++)
        cout << array[i] << " ";
    cout << endl;
}

void merge(int *array,int c, int sublen) {
    
    int l = 0;
    int m = c*sublen;
    int r = c*sublen+sublen;
    
//    cout<<m<<" "<<r<<endl;
    int i, j, k, nl, nr;
    nl = m-l; nr = r-m;
    
    int larr[nl], rarr[nr];

    for(i = 0; i<nl; i++)
        larr[i] = array[l+i];
    for(j = 0; j<nr; j++)
        rarr[j] = array[m+j];

//    display(larr,nl);
//    display(rarr,nr);
    
    i = 0; j = 0; k = l;
    //marge temp arrays to real array
    while(i < nl && j<nr) {
        if(larr[i] <= rarr[j]) {
            array[k] = larr[i];
            i++;
        }else{
            array[k] = rarr[j];
            j++;
        }
        k++;
    }
    
    while(i<nl) {       //extra element in left array
        array[k] = larr[i];
        i++; k++;
    }
    
    while(j<nr) {     //extra element in right array
        array[k] = rarr[j];
        j++; k++;
    }
    
//    display(array,len);
    
}

int main(int argc, char **argv){
    
    int rank, size;
    
    int unsorted[len];
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);
    srand(time(NULL));

    int sublen = len/(size-1);
    int sublist[sublen];
    
    if(rank){   // slaves
        MPI_Recv(&sublist,sublen,MPI_INT,0,0,MCW,MPI_STATUS_IGNORE);
        
        cout<<"Before sort p"<<rank<<": ";
        for(int i=0;i<sublen; i++){
            cout<<sublist[i]<<" ";
        }
        cout<<"\n";
        
        sort(sublist, sublist+sublen);
        
        cout<<"After sorted p"<<rank<<": ";
        for(int i=0;i<sublen; i++){
            cout<<sublist[i]<<" ";
        }
        cout<<"\n";
        
        MPI_Send(&sublist, sublen, MPI_INT, 0, 0, MCW);

    }else{   // master
        
        //Randomly genarate a list
        cout<<"Randomly generate a list: ";
        for(int i=0; i<len; i++){
            unsorted[i] = rand()%50;
            cout<< unsorted[i]<<" ";
        }
        cout<<"\n\n";
        
        //Send work to other processes
        for(int dest = 0; dest<size-1; dest++){
            MPI_Send(&unsorted[dest*sublen], sublen, MPI_INT, dest+1, 0, MCW);
        }

        int count = 0; //signal for merge
        
        //Recieve sorted list from other processes and merge those together
        for(int i = 0; i<size-1;i++){
            MPI_Recv(&unsorted[i*sublen],sublen, MPI_INT, i+1,0,MCW,MPI_STATUS_IGNORE);
            count++;
            if(count%2 == 0){
                merge(unsorted,i,sublen);
                count--;
                
                cout<<"\n\n";
                cout<<"After merge "<< i-1<<": ";
                display(unsorted,len);
                
            }
        }
        
        cout<<"\n\n";
        cout<<"Sorted list:";
        display(unsorted,len);
    }
    
    
    MPI_Finalize();

    return 0;
}
