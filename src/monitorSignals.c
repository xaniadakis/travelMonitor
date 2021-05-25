#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h> 
#include <sys/stat.h>
#include <sys/types.h>

#ifndef myLibHeader
#define myLibHeader
#include "../lib/headers/myLib.h"
#endif

#ifndef monitorUtilHeader
#define monitorUtilHeader
#include "../lib/headers/monitorUtil.h"
#endif

#define LOG_DIR "../log_folder/"
#define PURPLE  "\x1B[38;2;15;15;255m"    
#define RESET   "\x1b[0m"

extern int FLAGINTQUIT;
extern int FLAGUSR1;
extern int FLAGUSR2;
extern int FLAGCONT;

void monitorInterrupt(int signo)
{
    if(signo==SIGINT || signo==SIGQUIT){
        printf(PURPLE"Process %d will create log_folder.%d\n"RESET,getpid(),getpid());
        FLAGINTQUIT = 1;
    }
    else if(signo==SIGUSR1){
        printf(PURPLE"Process %d received SIGUSR1 and will reparse.\n"RESET,getpid());
        FLAGUSR1 = 1;
    }
    // else if(signo==SIGUSR2){
    //     printf(PURPLE"Process %d received SIGUSR2 and will send ready message if its ready.\n"RESET,getpid());
    //     FLAGUSR2 = 1;
    // }
}

void checkFlag(monitorDataPointer* Universal)
{
    //usleep_hook(1100); //yparxei gia na dwsei xrono sto signal na orisei to flag se periptwsh pou topothetithei amesws meta apo kill(pid)    
    int Ncountries = *(Universal->NcountriesPtr);
    char **countryData  = *(Universal->countryDataPtr);  
    monitorDir *mySubDir = *(Universal->mySubDirPtr); 
    
    if(FLAGCONT){
        FLAGCONT = 0;
        return;
    }
    if(FLAGINTQUIT)
    {
        mkdir(LOG_DIR, 0777);
        int file;
        int pid = getpid();
        char *filename = malloc(countDigits(pid)+10+strlen(LOG_DIR));
        snprintf(filename, countDigits(pid)+10+strlen(LOG_DIR) , LOG_DIR"log_file.%d", pid);
        if((file = creat(filename, 0777))<0) 
        {
            perror("creat");
            exit(1); 
        }     
        int fd = dup(1);
        if(dup2(file,1)<0) 
        {
            perror("dup2"); 
            exit(1);
        }
        for(int i=0;i<Ncountries;i++)
            printf("%s\n",countryData[i]);
        getRequests(Universal, NULL, NULL, NULL, NULL, 1);
        dup2(fd, 1);
        close(fd);
        free(filename);
        FLAGINTQUIT = 0;
    }
    else if(FLAGUSR1)
    {
        for(int i=0;i<mySubDir->Nsub_dirs;i++)
            parseDir(Universal, mySubDir->sub_dir[i]);
        storeDataIntoDatatypes(Universal);
        sendBloomFilters(Universal);
        kill(getpid(),SIGSTOP); 
        sendReadyMsg(Universal);     
        kill(getpid(),SIGSTOP); 
        FLAGUSR1=0;
    }
    // else if(FLAGUSR2)
    // {
    //     sendReadyMsg(Universal);
    //     FLAGUSR2=0;
    // }
    // FILE *file;
    // file = fopen(filename, "w+");
    // for(int i=0;i<Ncountries;i++)
    //     fprintf(file,"%s\n",countryData[i]);
    // fprintf(file,"TOTAL TRAVEL REQUESTS %d\n",totalTravelRequests);
    // fprintf(file,"ACCEPTED %d\n",acceptedRequests);
    // fprintf(file,"REJECTED %d\n",rejectedRequests);
    // fclose(file);
}