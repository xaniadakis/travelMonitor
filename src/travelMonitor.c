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
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>


#define FAILURE 1
#define SUCCESS 0
#define FIFO_DIR "../tmp/namedPipes/"
#define LOG_DIR "../log_folder/"
#define _XOPEN_SOURCE 700

#ifndef myLibHeader
#define myLibHeader
#include "../lib/headers/myLib.h"
#endif

#ifndef travelMonitorUtilHeader
#define travelMonitorUtilHeader
#include "../lib/headers/travelMonitorUtil.h"
#endif

#ifndef signalsHeader
#define signalsHeader
#include "../lib/headers/parentSignals.h"
#endif

extern int errno;
int FLAGINTQUIT = 0;
int FLAGCHLD = 0;
int FLAGUSR2 = 0;

int main(int argc, char *argv[])
{
    // system("clear");
    printf("\033[1A"); // Move up X lines;
    printf("\r\033[1;31mtravelMonitor\033[0m:\033[1;36m~/\033[1;39mLoading...\033[0m \n");

    parentDataPointer *Universal = NULL;   
    reqReg *requestsRegistry = NULL; 
    Monitor **monitor = NULL; 
    char *input_dir = NULL;
    char **namedPipe = NULL; 
    char **argument = NULL; 
    int *fifo = NULL; 
    int sizeOfBloom = 0;                          
    int numMonitors = 0;
    int bufferSize = 0;
    
    static struct sigaction act;
	act.sa_handler = parentInterrupt;
    act.sa_flags = SA_RESTART;
	sigfillset(&(act.sa_mask));
	sigaction(SIGINT, &act, NULL);
    sigaction(SIGQUIT, &act, NULL);
	sigaction(SIGCHLD, &act, NULL);
    sigaction(SIGUSR2, &act, NULL);

    //keep pointers to the important data of the program in a struct 
    uploadParentData(&Universal, &numMonitors, &input_dir, &sizeOfBloom, 
                     &bufferSize, &monitor, &act, &fifo, &namedPipe, 
                     &requestsRegistry, &argument);

    //check the parameters for correctness                             
    parametersCheck(Universal, argc, argv);

    //do some initializations
    initMonitorDetails(Universal);

    //delete temp files from previous execution
    deleteOldLogFiles();

    //create named pipes
    createNamedPipes(Universal);

    //create proccesses
    for(int i=0;i<numMonitors;i++) 
        fork_monitor(Universal, i, argument);

    //pass bufferSize, sizeOfbloom and input_dir toe proccesses
    for(int i=0;i<numMonitors;i++)
        passInfo(Universal, i);

    //distribute the subdir from the given input_dir to the proccesses using RR
    distributeSubdirs(Universal, -1);
    
    for(int i=0;i<numMonitors;i++) 
        for(int k=0;k<monitor[i]->Nsub_dirs;k++){
            printf("%d %s\n",k,monitor[i]->sub_dir[monitor[i]->Nsub_dirs]);
        }
    
    //wait while monitor proccesses create the datatypes
    usleep_hook(110000);     

    //receive the bloomfilters from monitor proccesses
    for(int i=0;i<numMonitors;i++)
        receiveBloomFilter(Universal, i);

    //receive ready message from monitors
    for(int i=0;i<numMonitors;i++)
        kill(monitor[i]->pid,SIGCONT);
    for(int i=0;i<numMonitors;i++){
        while(!receiveReadyMsg(Universal, i))
            kill(monitor[i]->pid,SIGCONT);
    }
    for(int i=0;i<numMonitors;i++)
        kill(monitor[i]->pid,SIGCONT);

    //wait for user input from keyboard 
    readInputFromKeyboard(Universal);

    //delete temp files
    rmdir("../tmp/namedPipes");
    rmdir("../tmp");

    //exit normally
    return SUCCESS;
}
