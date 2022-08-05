#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <omp.h>
#include <math.h>
#include "functions.h"

BOOL readNumbersFromFile(double *matchingValue, int *sizeOfPictures, int *sizeOfObjects, Object **pictures, Object **objects)
{
    int successReadNumbers = FALSE;
    FILE *fptr;
    if ((fptr = fopen(INPUT_FILE_NAME, "r")) == 0)
        fprintf(stderr, "openfile failed\n");

    fscanf(fptr, "%lf", matchingValue);
    fscanf(fptr, "%d", sizeOfPictures);

    *pictures = (Object *)doMalloc(*sizeOfPictures * sizeof(Object));
    createMatrixes(fptr, *sizeOfPictures, *pictures);

    fscanf(fptr, "%d", sizeOfObjects);
    *objects = (Object *)doMalloc(*sizeOfObjects * sizeof(Object));
    createMatrixes(fptr, *sizeOfObjects, *objects);

    successReadNumbers = TRUE;

    fclose(fptr);
    return successReadNumbers;
}

Object *createMatrixes(FILE *fptr, int size, Object *type)
{
    for (int i = 0; i < size; i++)
    {
        fscanf(fptr, "%d", &type[i].id);
        fscanf(fptr, "%d", &type[i].dimension);

        type[i].elements = (int **)doMalloc(type[i].dimension * sizeof(int *));
        for (int j = 0; j < type[i].dimension; j++)
        {
            type[i].elements[j] = (int *)doMalloc(type[i].dimension * sizeof(int));
            for (int k = 0; k < type[i].dimension; k++)
                fscanf(fptr, "%d", &type[i].elements[j][k]);
        }
    }
    return type;
}

void createInfoFromFileType(MPI_Datatype *dataType)
{
    MPI_Type_contiguous(5, MPI_INT, dataType);
    MPI_Type_commit(dataType);
}

void *doMalloc(unsigned int nbytes)
{
    void *p = malloc(nbytes);

    if (p == NULL)
    {
        fprintf(stderr, "malloc failed\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    return p;
}

void sendData(int size, Object *type)
{
    for (int i = 0; i < size; i++)
    {
        MPI_Send(&type[i].id, 1, MPI_INT, WORKER, 0, MPI_COMM_WORLD);
        MPI_Send(&type[i].dimension, 1, MPI_INT, WORKER, 0, MPI_COMM_WORLD);
        for (int j = 0; j < type[i].dimension; j++)
            for (int k = 0; k < type[i].dimension; k++)
                MPI_Send(&type[i].elements[j][k], 1, MPI_INT, WORKER, 0, MPI_COMM_WORLD);
    }
}

Object *recvData(int size)
{
    Object *type = (Object *)doMalloc(size * sizeof(Object));
    for (int i = 0; i < size; i++)
    {
        MPI_Recv(&type[i].id, 1, MPI_INT, MASTER, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&type[i].dimension, 1, MPI_INT, MASTER, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        type[i].elements = (int **)doMalloc(type[i].dimension * sizeof(int *));
        for (int j = 0; j < type[i].dimension; j++)
        {
            type[i].elements[j] = (int *)doMalloc(type[i].dimension * sizeof(int));
            for (int k = 0; k < type[i].dimension; k++)
                MPI_Recv(&type[i].elements[j][k], 1, MPI_INT, MASTER, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }
    return type;
}

void checkMatchingValue(Exist **existArray, int sizePic, int sizeObj, double matchingValue, Object *pictures, Object *objects)
{
    Exist *tempExistArray;
    int offsetSize;
    double tempMatchingValue = 0.0;

    tempExistArray = (Exist *)doMalloc(sizeof(Exist) * sizePic);

    for (int a = 0; a < sizePic; a++)
    {
        tempExistArray[a].isExist = FALSE;
        tempExistArray[a].idPic = pictures[a].id;

        for (int b = 0; b < sizeObj && !tempExistArray[a].isExist; b++)
        {
            offsetSize = pictures[a].dimension - objects[b].dimension;
#pragma omp parallel for firstprivate(tempMatchingValue)
            for (int c = 0; c <= offsetSize; c++)
            {
                for (int d = 0; d <= offsetSize && !tempExistArray[a].isExist; d++)
                {
                    tempMatchingValue = 0.0;
                    for (int e = 0; e < objects[b].dimension; e++)
                    {
                        for (int f = 0; f < objects[b].dimension; f++)
                            tempMatchingValue += fabs((double)(pictures[a].elements[e + c][f + d] - objects[b].elements[e][f]) / (double)pictures[a].elements[e + c][f + d]);
                    }
                    if (tempMatchingValue <= matchingValue)
                    {
                        tempExistArray[a].idPic = pictures[a].id;
                        tempExistArray[a].idObj = objects[b].id;
                        tempExistArray[a].row = c;
                        tempExistArray[a].col = d;
                        tempExistArray[a].isExist = TRUE;
                    }
                }
            }
        }
    }
    *existArray = tempExistArray;
}

void checkMatchingValueSequential(Exist **existArray, int sizePic, int sizeObj, double matchingValue, Object *pictures, Object *objects)
{
    Exist *tempExistArray;
    int offsetSize;
    double tempMatchingValue = 0.0;

    tempExistArray = (Exist *)doMalloc(sizeof(Exist) * sizePic);

    for (int a = 0; a < sizePic; a++)
    {
        tempExistArray[a].isExist = FALSE;
        tempExistArray[a].idPic = pictures[a].id;

        for (int b = 0; b < sizeObj && !tempExistArray[a].isExist; b++)
        {
            offsetSize = pictures[a].dimension - objects[b].dimension;
            for (int c = 0; c <= offsetSize && !tempExistArray[a].isExist; c++)
            {
                for (int d = 0; d <= offsetSize && !tempExistArray[a].isExist; d++)
                {
                    tempMatchingValue = 0.0;
                    for (int e = 0; e < objects[b].dimension; e++)
                    {
                        for (int f = 0; f < objects[b].dimension; f++)
                            tempMatchingValue += fabs((double)(pictures[a].elements[e + c][f + d] - objects[b].elements[e][f]) / (double)pictures[a].elements[e + c][f + d]);
                    }
                    if (tempMatchingValue <= matchingValue)
                    {
                        tempExistArray[a].idPic = pictures[a].id;
                        tempExistArray[a].idObj = objects[b].id;
                        tempExistArray[a].row = c;
                        tempExistArray[a].col = d;
                        tempExistArray[a].isExist = TRUE;
                    }
                }
            }
        }
    }
    *existArray = tempExistArray;
}

void printResult(int sizeExist, Exist *exist)
{
    for (int i = 0; i < sizeExist; i++)
    {
        if (exist[i].isExist == TRUE)
            printf("Picture %d: found Object %d in Position(%d,%d)\n", exist[i].idPic, exist[i].idObj, exist[i].row, exist[i].col);
        else
            printf("Picture %d: No Objects were found\n", exist[i].idPic);
    }
}

void freeAllocateMemory(int sizeElement, Object *type)
{
    for (int i = 0; i < sizeElement; i++)
    {
        for (int j = 0; j < type[i].dimension; j++)
            free(type[i].elements[j]);
        free(type[i].elements);
    }
}
