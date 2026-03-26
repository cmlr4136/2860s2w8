//
// Starting code for the portfolio exercise. Some required routines are included in a separate
// file (ending '_extra.h'); this file should not be altered, as it will be replaced with a different
// version for assessment.
//
// Compile as normal, e.g.,
//
// > gcc -o portfolioExercise portfolioExercise.c
//
// and launch with the problem size N and number of threads p as command line arguments, e.g.,
//
// > ./portfolioExercise 12 4
//


//
// Includes.
//
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#include "portfolioExercise_extra.h"        // Contains routines not essential to the assessment.

typedef struct { // Custom backpack data structure to pass arguements to threads
    int startRow; // First row
    int endRow; // Row where calculation stops
    int N; // Size
    float **M; // Pointer to 2D matrix M
    float *u; // Pointer to vector U
    float *v; // Pointer to vector V
    float *localDotProduct; // Where thread will store its answer
} threadData_t;

void* parallel_calc(void* arg) { // Function threads run
    threadData_t *data = (threadData_t*)arg; // Cast data to data structure
    for (int row = data->startRow; row < data->endRow; row++) { // Loop from start row to end row for this thread
        data->v[row] = 0.0f; // Wipe rows answer
        for (int col = 0; col < data->N; col++) { // Go through each column in the row
            data->v[row] += data->M[row][col] * data->u[col]; // Matrix element * Vector element and add to total
        }
    }
    *(data->localDotProduct) = 0.0f; // This Threads dot tally make 0
    for (int i = data->startRow; i < data->endRow; i++) { // Rows covered are looped 
        *(data->localDotProduct) += data->v[i] * data->v[i]; // Vector element squared added to threads local tally
    }
    return NULL;
}

//
// Main.
//
int main( int argc, char **argv )
{
    //
    // Initialisation and set-up.
    //

    // Get problem size and number of threads from command line arguments.
    int N, nThreads;
    if( parseCmdLineArgs(argc,argv,&N,&nThreads)==-1 ) return EXIT_FAILURE;

    // Initialise (i.e, allocate memory and assign values to) the matrix and the vectors.
    float **M, *u, *v;
    if( initialiseMatrixAndVector(N,&M,&u,&v)==-1 ) return EXIT_FAILURE;

    // For debugging purposes; only display small problems (e.g., N=8 and nThreads=2 or 4).
    if( N<=12 ) displayProblem( N, M, u, v );

    // Start the timing now.
    struct timespec startTime, endTime;
    clock_gettime( CLOCK_REALTIME, &startTime );

    //
    // Parallel operations, timed.
    //
    float dotProduct = 0.0f;        // You should leave the result of your calculation in this variable.
    
    pthread_t *threads = malloc(nThreads * sizeof(pthread_t)); // Memory to keep track of threads
    threadData_t *threadArgs = malloc(nThreads * sizeof(threadData_t)); // Memory for the argument of each thread
    float *localDotProducts = malloc(nThreads * sizeof(float)); // Array to hold individual results
    int rowsPerThread = N / nThreads; // Rows each thread gets
    for (int i = 0; i < nThreads; i++) {  //Loop for creating threads and launching them
        threadArgs[i].startRow = i * rowsPerThread;
        threadArgs[i].endRow = (i + 1) * rowsPerThread;
        threadArgs[i].N = N; // Give matrix size
        threadArgs[i].M = M; // Give matrix
        threadArgs[i].u = u; // Give vector u
        threadArgs[i].v = v; // Give vector V
        threadArgs[i].localDotProduct = &localDotProducts[i]; // Point thread to its specific slot
        pthread_create(&threads[i], NULL, parallel_calc, &threadArgs[i]); // Launch thread
    }
    for (int i = 0; i < nThreads; i++) { // Need to wait for threads to finish calculating
        pthread_join(threads[i], NULL); // Forces program to pause until specific thread is complete
        dotProduct += localDotProducts[i]; // Add local answer to total
    }
    if( N<=12 ) displayProblem( N, M, u, v );
    free(threads); // Clean memory
    free(threadArgs);
    free(localDotProducts);

    // DO NOT REMOVE OR MODIFY THIS PRINT STATEMENT AS IT IS REQUIRED BY THE ASSESSMENT.
    printf( "Result of parallel calculation: %f\n", dotProduct );

    //
    // Check against the serial calculation.
    //

    // Output final time taken.
    clock_gettime( CLOCK_REALTIME, &endTime );
    double seconds = (double)( endTime.tv_sec + 1e-9*endTime.tv_nsec - startTime.tv_sec - 1e-9*startTime.tv_nsec );
    printf( "Time for parallel calculations: %g secs.\n", seconds );

    // Step 1. Matrix-vector multiplication Mu = v.
    for( int row=0; row<N; row++ )
    {
        v[row] = 0.0f;              // Make sure the right-hand side vector is initially zero.

        for( int col=0; col<N; col++ )
            v[row] += M[row][col] * u[col];
    }

    // Step 2: The dot product of the vector v with itself
    float dotProduct_serial = 0.0f;
    for( int i=0; i<N; i++ ) dotProduct_serial += v[i]*v[i];

    // DO NOT REMOVE OR MODIFY THIS PRINT STATEMENT AS IT IS REQUIRED BY THE ASSESSMENT.
    printf( "Result of the serial calculation: %f\n", dotProduct_serial );

    //
    // Clear up and quit.
    //
    freeMatrixAndVector( N, M, u, v );

    return EXIT_SUCCESS;
}