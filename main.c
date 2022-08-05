#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <omp.h>
#include "functions.h"

int main(int argc, char *argv[])
{
    int rank, numProcs, sizeOfPictures, sizeOfObjects, halfSize, masterSize, position = 0;
    double time, matchingValue;
    char buff[BUFFER_SIZE];
    Object *pictures, *objects;
    Exist *existArray;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);

    if (numProcs != 2)
    {
        printf("Run the example with two processes only\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    // create mpi data type the send and recv with mpi
    MPI_Datatype ExistMPIType;
    createInfoFromFileType(&ExistMPIType);

    if (rank == MASTER)
    {
        if (readNumbersFromFile(&matchingValue, &sizeOfPictures, &sizeOfObjects, &pictures, &objects) == FALSE)
        {
            fprintf(stderr, "read from file failed1\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }

        halfSize = sizeOfPictures / 2;
        masterSize = halfSize + sizeOfPictures % 2;
        time = MPI_Wtime();

        existArray = (Exist *)doMalloc(sizeof(Exist) * sizeOfPictures);

        // we want to send A large amount of variables at once
        MPI_Pack(&matchingValue, 1, MPI_DOUBLE, buff, BUFFER_SIZE, &position, MPI_COMM_WORLD);
        MPI_Pack(&halfSize, 1, MPI_INT, buff, BUFFER_SIZE, &position, MPI_COMM_WORLD);
        MPI_Pack(&sizeOfObjects, 1, MPI_INT, buff, BUFFER_SIZE, &position, MPI_COMM_WORLD);
        MPI_Send(buff, position, MPI_PACKED, WORKER, 0, MPI_COMM_WORLD);

        sendData(halfSize, pictures + masterSize);
        sendData(sizeOfObjects, objects);

        // create array of exist to check what picture have a match of object (master)
        checkMatchingValue(&existArray, masterSize, sizeOfObjects, matchingValue, pictures, objects);

        // recv existArray from slave to check if found objects in picture in the second half that we send
        MPI_Recv(existArray + masterSize, halfSize, ExistMPIType, WORKER, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        printf("Parallel Version run time: %lf\n", MPI_Wtime() - time);
        printResult(sizeOfPictures, existArray);

        time = MPI_Wtime();

        // create array of exist to check what picture have a match of object
        checkMatchingValueSequential(&existArray, sizeOfPictures, sizeOfObjects, matchingValue, pictures, objects);

        printf("\nSequential Version run time: %lf\n", MPI_Wtime() - time);
        printResult(sizeOfPictures, existArray);
    }
    else
    {
        // recv the buffer that contain the large amount of variables from master
        MPI_Recv(buff, BUFFER_SIZE, MPI_PACKED, MASTER, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        MPI_Unpack(buff, BUFFER_SIZE, &position, &matchingValue, 1, MPI_DOUBLE, MPI_COMM_WORLD);
        MPI_Unpack(buff, BUFFER_SIZE, &position, &sizeOfPictures, 1, MPI_INT, MPI_COMM_WORLD);
        MPI_Unpack(buff, BUFFER_SIZE, &position, &sizeOfObjects, 1, MPI_INT, MPI_COMM_WORLD);

        pictures = recvData(sizeOfPictures);
        objects = recvData(sizeOfObjects);

        existArray = (Exist *)doMalloc(sizeof(Exist) * sizeOfPictures);

        // check if picture have match of object from the half size of array of picture
        checkMatchingValue(&existArray, sizeOfPictures, sizeOfObjects, matchingValue, pictures, objects);

        MPI_Send(existArray, sizeOfPictures, ExistMPIType, MASTER, 0, MPI_COMM_WORLD);
    }

    free(existArray);
    freeAllocateMemory(sizeOfPictures, pictures);
    freeAllocateMemory(sizeOfObjects, objects);
    MPI_Finalize();
    return EXIT_SUCCESS;
}
