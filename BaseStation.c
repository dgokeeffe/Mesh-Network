/***************************************************************************************************************
 *                          Base Station for Wireless Sensor Network Simulator
 *  File        : BaseStation.c
 *  Description : Simulates a 4x15 fully distributed WSN in openmpi
 *  Parameters  :
 *  Returns     :
 **************************************************************************************************************/ 

/***************************************************************************************************************
 *                                              HEADERS
 **************************************************************************************************************/ 
#include <stdio.h>  //for printf
#include <mpi.h>    //for mpi
#include <stdlib.h> //for psuedo-random number generation
#include <time.h>
#include <assert.h>
/***************************************************************************************************************
 *                                              DEFINITIONS
 **************************************************************************************************************/ 
#define SIZE  60 
#define NUM_RETURN_NODES 42
#define WIRELESS_SENSOR_NODE "WSN"

/***************************************************************************************************************
 *                                              FUNCTIONS
 **************************************************************************************************************/

int main( int argc, char *argv[] )
{
    // initiate buffers and variables
    int numnodes=SIZE, events = 0;
    int nodebuf={MPI_PROC_NULL,};
    int quadrant[NUM_RETURN_NODES] = {0}; 

    // initiate MPI communicator
    MPI_Comm workercomm;

    // initiate MPI
    MPI_Init( &argc, &argv );

    // make the WSN 
    if (MPI_Comm_spawn(WIRELESS_SENSOR_NODE, MPI_ARGV_NULL, numnodes, MPI_INFO_NULL,
                0, MPI_COMM_WORLD, &workercomm, MPI_ERRCODES_IGNORE ) != MPI_SUCCESS)
        fprintf(stderr, "Spawn failed!\n");
    else{
        // start timing total execution
        double t1 = MPI_Wtime();
        // receive events from the slaves (-1 denotes end of simulation which runs for 1000 steps)
        while (nodebuf != -1){ 
            MPI_Recv(&nodebuf, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, workercomm, MPI_STATUS_IGNORE);
            // the last -1 for the end is received so don't print that
            if (nodebuf != -1) {
                // a node is skipped every row so the quadrant number has to be incremented accordingly
                if (nodebuf < 30){ 
                    quadrant[nodebuf - 16]++; 
                } else if ((nodebuf > 30) && (nodebuf < 45)){
                    quadrant[nodebuf - 17]++; 
                } else {
                    quadrant[nodebuf - 18]++; 
                }
                events++;
            } else {
                // print stats
                double t2 = (MPI_Wtime()-t1);
                printf("Total execution time: %f seconds\n", t2);
                printf("The events detected are: \n");
                for (int i = 0; i < NUM_RETURN_NODES; i++){
                    printf("Quadrant[%d] - %d events\n", i, quadrant[i]);
                }
                printf("The total number of events was: %d\n", events);
            }
        }
    }
    MPI_Finalize();
    return 0;
}
