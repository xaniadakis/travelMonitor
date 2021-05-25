#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/select.h>
#include <ctype.h>

#ifndef myLibHeader
#define myLibHeader
#include "../lib/headers/myLib.h"
#endif

#ifndef DataStructHeader
#define DataStructHeader
#include "../lib/structures/DataStruct.h"
#endif

#ifndef hashTableHeader
#define hashTableHeader
#include "../lib/headers/hashTable.h"
#endif

#ifndef skipListHeader
#define skipListHeader
#include "../lib/headers/skipList.h"
#endif

#ifndef monitorUtilHeader
#define monitorUtilHeader
#include "../lib/headers/monitorUtil.h"
#endif

#ifndef hashFunctionsHeader
#define hashFunctionsHeader
#include "../lib/headers/hashFunctions.h"
#endif

#ifndef signalsHeader
#define signalsHeader
#include "../lib/headers/monitorSignals.h"
#endif

#define READ 0
#define WRITE 1
#define _XOPEN_SOURCE 700

int FLAGINTQUIT = 0;
int FLAGUSR1 = 0;
int FLAGUSR2 = 0;
int FLAGCONT = 0;

int main(int argc, char *argv[])
{
    // if(argc>2) printf("Process %d   \"%s\"  \"%s\"  %s\n",getpid(),argv[1],argv[2],argv[3]);
    static struct sigaction act;
	act.sa_handler = monitorInterrupt;
    act.sa_flags = SA_RESTART;
	sigfillset(&(act.sa_mask));
	sigaction(SIGINT, &act, NULL);
    sigaction(SIGQUIT, &act, NULL);
    sigaction(SIGUSR1, &act, NULL);
    sigaction(SIGUSR2, &act, NULL);

    //dataTypes
    monitorDataPointer *Universal = NULL;       //a struct that keeps pointers to the important data of the program   
    Record **Registry = NULL;                   //hashTable dataType
    Sentinel **skipList = NULL;                 //skipList dataType
    char **bloomFilter = NULL;                  //bloomFilter dataType 
    reqReg *requestsRegistry = NULL;            //a struct to save the requests happening
    monitorDir *mySubDir = NULL;                //a struct to save info about files and sub_dirs assigned to this monitor

    //pluralities
    int sizeOfBloom = 0;                        //size of bloomFilter in bytes (defined by the user)
    int inputSize = 0;                          //plurality of citizens kept in the dataBase (no duplicates)
    int NskipLists = 0;                         //plurality of skipLists
    int NbloomFilters = 0;                      //plurality of bloomFilters == plurality of viruses stored in array virusData
    int Ncountries = 0;                         //plurality of countries stored in array countryData
    int bufferSize = 0;                         //size of buffer for read and write on named-pipes

    //arrays that store data to avoid information reoccurrence
    char **virusData = NULL;                    //virusData array keeps track of the viruses that exist
    char **countryData = NULL;                  //countryData array keeps track of the countries that exist
    char **vacced = NULL;                       //vacced array has two items "YES" and "NO"
    char *input_dir = NULL;      

    //named pipes
    int *fifo = NULL;

    //save pointers to all the important data of the program in a struct for easy access
    uploadMonitorData(&Universal, &Registry, &skipList, &bloomFilter, &countryData,
                       &virusData, &vacced, &input_dir, &bufferSize, &NbloomFilters, &NskipLists, 
                       &sizeOfBloom, &Ncountries, &inputSize, &mySubDir, &fifo, &requestsRegistry, argv);

    //receive bufferSize, sizeOfBloom and input_dir                
    receiveInfo(Universal);

    //receive from parent the subdirectories you will parse
    receiveSubdirs(Universal);

    //store the records inside skipLists and bloomFilters
    storeDataIntoDatatypes(Universal); 

    //send the bloomFilters
    sendBloomFilters(Universal);

    //send READY message
    kill(getpid(),SIGSTOP); 
    sendReadyMsg(Universal);     
    kill(getpid(),SIGSTOP);

    //wait for commands from parent
    waitForParent(Universal);

    //is not going to be executed, since monitor receives SIGKILL when parent is done
    close(fifo[WRITE]);
    close(fifo[READ]);
    exitNow(&Universal);
    printf("Proccess %d is exiting normally\n", getpid());
    return 0;
}