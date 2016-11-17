/***************************************************************************************************************
 *                                      Wireless Sensor Network Simulator
 *  File        : WSN.c
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
#include <time.h>   //to seed the random number generator
/***************************************************************************************************************
 *                                              DEFINITIONS
 **************************************************************************************************************/ 
#define SIZE  60 
#define UP    0
#define DOWN  1
#define LEFT  2
#define RIGHT 3
#define SIMULATION_SIZE 1000
#define SET_COLUMN 1
#define SET_ROW 0
#define RIGHT_OR_DOWN 1
#define LEFT_OR_UP 0 
#define RAND_RANGE 10

/***************************************************************************************************************
 *                                              FUNCTIONS
 **************************************************************************************************************/ 
/* Must run on 60 slave processes */

int main(int argc, char *argv[])  
{
    int numtasks, rank, source, dest, tag=1, nbrs[4], dims[2]={4,15}, 
        periods[2]={0,0}, reorder=0, coords[2];

    // initalize buffers
    int leftbuf[2]={MPI_PROC_NULL, MPI_PROC_NULL,}, downbuf={MPI_PROC_NULL,}, 
    inleftbuf[2]={MPI_PROC_NULL, MPI_PROC_NULL,}, masterbuf={MPI_PROC_NULL,};

    // Initialize MPI and get tasks and rank
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    MPI_Comm cartcomm;
    MPI_Comm intercomm;

    // Get the intercommunicator 
    // The manager is represented as the process with rank 0 in (the remote group of) MPI_COMM_PARENT.  
    // Spawn processes can communicate amidst themselves using MPI_COMM_WORLD
    MPI_Comm_get_parent(&intercomm);

    if (numtasks == SIZE) {
        // Create the Cartesian 4x15 grid topology
        MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, reorder, &cartcomm);
        // Give each process in the grid its rank
        MPI_Comm_rank(cartcomm, &rank);
        // Give each process in the grid its coordinates 
        MPI_Cart_coords(cartcomm, rank, 2, coords);

        // Shift the Virtual Topology so that the adjacent nodes can be called 
        // By using only nbrs to set the dest and source the adajacent node req. is met
        MPI_Cart_shift(cartcomm, 0, 1, &nbrs[UP], &nbrs[DOWN]);
        MPI_Cart_shift(cartcomm, 2, 1, &nbrs[LEFT], &nbrs[RIGHT]);

        // run the simulation for 1000 steps
        for (int j=0; j<SIMULATION_SIZE; j++){
            // each node generates a random number between 0 and 9 (will result in hit 10% of the time)
            int randomBit = rand() % RAND_RANGE;
            // if in the first three rows, send your random number to the node below you
            if (coords[0] < 3){
                dest = nbrs[DOWN];
                // set the downbuf to the number you generated
                downbuf = randomBit;
                // send the random bit across
                MPI_Send(&downbuf, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
            }

            /* if in the last three rows, send the number you just received and the random number you generated 
               to the node to the right */
            if (coords[0] > 0 && coords[1] < 14){
                // set the source for the receive above
                source = nbrs[UP];
                // receive
                MPI_Recv(&leftbuf[0], 1, MPI_INT, source, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                // set the destination for the node to the right
                dest = nbrs[RIGHT];
                // put your random number into leftbuf to send
                leftbuf[1] = randomBit;
                // send
                MPI_Send(&leftbuf, 2, MPI_INT, dest, tag, MPI_COMM_WORLD);
            }

            /* if not in the first row or the first column, you now have all the nearest node numbers, so check
               if they are equal, if so, send to master */
            if (coords[0] > 0 && coords[1] != 0){
                // set the source for the receive to the left
                source = nbrs[LEFT];
                MPI_Recv(&inleftbuf, 2, MPI_INT, source, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                // now check all the numbers: down, self, left, left-up
                if ((leftbuf[0]) == (leftbuf[1]) == (inleftbuf[0]) == (inleftbuf[1])){
                    // send rank to master
                    masterbuf = rank;
                    MPI_Send(&masterbuf, 1, MPI_INT, 0, tag, intercomm);
                }
            }

            // Synchronize for next round 
            MPI_Barrier(MPI_COMM_WORLD);
        }
        if (rank == 0){
            // send to the master we're done
            masterbuf = -1;
            MPI_Send(&masterbuf, 1, MPI_INT, 0, tag, intercomm);
        }
    }
    else
        printf("Must specify %d processors. Terminating.\n",SIZE);
    MPI_Comm_free(&intercomm);
    MPI_Comm_free(&cartcomm);
    MPI_Finalize();
}
