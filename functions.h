#ifndef __FUNCTIONS_H__
#define __FUNCTIONS_H__

#define INPUT_FILE_NAME "input.txt"

typedef int BOOL;

enum bool
{
    FALSE,
    TRUE
};

enum buffer
{
    BUFFER_SIZE = 1 << 20
};

enum ranks
{
    MASTER,
    WORKER
};

typedef struct
{
    int id;
    int dimension;
    int **elements;
} Object;

typedef struct
{
    int idPic;
    int idObj;
    int row;
    int col;
    BOOL isExist;
} Exist;
/**
 * @brief read the data from file
 * @param matchingValue this is the matching value that we check if the calculation less than this value
 * @param sizeOfPictures the count of pictures in the file
 * @param sizeOfObjects the count of objects in the file
 * @param pictures array of pictures
 * @param objects array of objects
 * @return BOOL
 */
BOOL readNumbersFromFile(double *matchingValue, int *sizeOfPictures, int *sizeOfObjects, Object **pictures, Object **objects);
/**
 * @brief Create a Matrixes object
 * @param fptr pointer file that we read from him
 * @param size count of pictures or objects
 * @param type picture/object
 * @return Object*
 */
Object *createMatrixes(FILE *fptr, int size, Object *type);
/**
 * @brief create data type for mpi
 * @param dataType
 */
void createInfoFromFileType(MPI_Datatype *dataType);
/**
 * @brief allocate memory malloc version
 * @param nbytes
 * @return void*
 */
void *doMalloc(unsigned int nbytes);
/**
 * @brief send data from master to slave half of the array of pictures
 * @param size
 * @param type
 */
void sendData(int size, Object *type);
/**
 * @brief recv data from master to slave
 * @param size
 * @return Object*
 */
Object *recvData(int size);
/**
 * @brief version of parallel to the calculation and return the exist array
 * @param sizeExistArray
 * @param existArray
 * @param sizePic
 * @param sizeObj
 * @param matchingValue
 * @param pictures
 * @param objects
 * @return Exist*
 */
void checkMatchingValue(Exist **existArray, int sizePic, int sizeObj, double matchingValue, Object *pictures, Object *objects);
/**
 * @brief version of parallel to the calculation and return the exist array
 * @param existArray
 * @param sizePic
 * @param sizeObj
 * @param matchingValue
 * @param pictures
 * @param objects
 * @return Exist*
 */
void checkMatchingValueSequential(Exist **existArray, int sizePic, int sizeObj, double matchingValue, Object *pictures, Object *objects);
/**
 * @brief version of sequential to the calculation and return the exist array
 * @param existArray
 * @param sizePic
 * @param sizeObj
 * @param matchingValue
 * @param pictures
 * @param objects
 * @return Exist*
 */

void printResult(int sizeExistMaster, Exist *existMaster);
/**
 * @brief print the result of the parallel and sequential version
 * @param sizeExist
 * @param exist
 * @param sizeOfPictures
 * @param objectExistInPicture
 * @param pictures
 */
void freeAllocateMemory(int sizeElement, Object *type);
/**
 * @brief free memory pictures and objects
 */
#endif