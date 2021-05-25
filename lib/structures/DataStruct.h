#include <stdio.h>
#include <arpa/inet.h>
#include <signal.h>

#ifndef skipStructHeader
#define skipStructHeader
#include "skipStruct.h"
#endif

#ifndef hashTableStructHeader
#define hashTableStructHeader
#include "hashTableStruct.h"
#endif

#ifndef RequestStructHeader
#define RequestStructHeader
#include "requestStruct.h"
#endif

#ifndef moreStructsHeader
#define moreStructsHeader
#include "moreStructs.h"
#endif

typedef struct parentDataPointer{
    reqReg **requestsRegistryPtr;
    int *numMonitorsPtr;
    char **dirnamePtr;
    char ***argumentPtr;
    int *bloomSizePtr;
    int *bufferSizePtr;
    Monitor ***monitorPtr;
    struct sigaction *actPtr;
    int **fifoPtr;
    char ***namedPipePtr;
}parentDataPointer;

typedef struct monitorDataPointer{
    reqReg **requestsRegistryPtr;
    Record ***RegistryPtr;
    Sentinel ***skipListPtr;
    char ***bloomFilterPtr;
    char ***countryDataPtr;
    char ***virusDataPtr;
    char ***vaccedPtr;
    char **dirnamePtr;
    int *bufferSizePtr;
    int *NbloomFiltersPtr;
    int *NskipListsPtr;
    int *bloomSizePtr;
    int *NcountriesPtr;
    int *inputSizePtr;
    monitorDir **mySubDirPtr;
    int **fifoPtr;
}monitorDataPointer;


