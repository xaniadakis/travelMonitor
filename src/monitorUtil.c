#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <wchar.h>
#include <locale.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>


#ifndef monitorUtilHeader
#define monitorUtilHeader
#include "../lib/headers/monitorUtil.h"
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

#ifndef myLibHeader
#define myLibHeader
#include "../lib/headers/myLib.h"
#endif

#ifndef bloomFilterHeader
#define bloomFilterHeader
#include "../lib/headers/bloomFilter.h"
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

void uploadMonitorData(monitorDataPointer **Universal, Record ***Registry, Sentinel ***skipList, char ***bloomFilter, char ***countryData,
                       char ***virusData, char ***vacced, char **dirname, int *bufferSize, int *NbloomFilters, int *NskipLists, 
                       int *bloomSize, int *Ncountries, int *inputSize, monitorDir **mySubDir,
                       int **fifo, reqReg **requestsRegistry, char **argv)
{
    (*Universal) = (monitorDataPointer*) malloc(sizeof(monitorDataPointer));
    (*Universal)->RegistryPtr = Registry;
    (*Universal)->skipListPtr = skipList;
    (*Universal)->bloomFilterPtr = bloomFilter;
    (*Universal)->countryDataPtr = countryData;
    (*Universal)->virusDataPtr = virusData;
    (*Universal)->vaccedPtr = vacced;
    (*Universal)->dirnamePtr = dirname;
    (*Universal)->bufferSizePtr = bufferSize;
    (*Universal)->NbloomFiltersPtr = NbloomFilters;
    (*Universal)->NskipListsPtr = NskipLists;
    (*Universal)->bloomSizePtr = bloomSize;
    (*Universal)->NcountriesPtr = Ncountries;
    (*Universal)->inputSizePtr = inputSize;
    (*Universal)->mySubDirPtr = mySubDir;
    (*Universal)->requestsRegistryPtr = requestsRegistry;
    (*Universal)->fifoPtr = fifo;

    //allocate and nullify the hash table array (pointers to linkedLists)
    (*Registry) = (Record**) malloc(10 * sizeof(Record*));
    for(int i=0;i<10;i++)
		  (*Registry)[i] = NULL;
    //allocated the vaccinated array which stores the values "YES" or "NO" to avoid information reoccurence
    //the citizen records' vaccinated field will point towards this array
    (*vacced) = (char**) malloc(sizeof(char*)*2);
    (*vacced)[0] = (char*) malloc(sizeof(char)*3);
    strcpy((*vacced)[0], "NO");
    (*vacced)[1] = (char*) malloc(sizeof(char)*4);
    strcpy((*vacced)[1], "YES");

    if(mySubDir==NULL)  return;
    (*mySubDir) = (monitorDir*) malloc_hook(sizeof(monitorDir));
    (*mySubDir)->Nsub_dirs = 0;
    (*mySubDir)->Nread_files = 0;
    (*mySubDir)->sub_dir = NULL;
    (*mySubDir)->read_files = NULL;

    (*requestsRegistry) = malloc_hook(sizeof(reqReg));
    (*requestsRegistry)->n = 0;
    (*requestsRegistry)->array = NULL;

    (*fifo) = (int*) malloc_hook(sizeof(int)*2);
    (*fifo)[READ] = open(argv[1], O_RDONLY);
    if((*fifo)[READ]<0) 
        perror("fifo");
    (*fifo)[WRITE] = open(argv[2], O_WRONLY);
    if((*fifo)[WRITE]<0) 
        perror("fifo");
}

void receiveInfo(monitorDataPointer *Universal)
{
    int *bufferSize = Universal->bufferSizePtr; 
    char **dirname = Universal->dirnamePtr;
    int *bloomSize = Universal->bloomSizePtr; 
    int *fifo = *(Universal->fifoPtr); 
    void *receivedBuffer = malloc_hook(1);
    argvs *pass_arg;
    //sleep(2);
    //if(read_hook(fifo[READ],&receivedBuffer,1)!=NULL){
    if(read_hook(fifo[0],&receivedBuffer,1)!=0){
        pass_arg = (argvs*) receivedBuffer;             
        (*bufferSize) = pass_arg->bufferSize;  
        (*bloomSize) = pass_arg->bloomSize;      
    }
    else
        printf("received nothing\n");
    //if(read_hook(fifo[READ],&receivedBuffer,1)!=NULL){
    if(read_hook(fifo[0],&receivedBuffer,1)!=0){
        (*dirname) = (char*) malloc_hook(strlen(receivedBuffer)+1);            
        strcpy((*dirname), receivedBuffer); 
    }
    else
        printf("received nothing\n");
    free(receivedBuffer);
}

void receiveSubdirs(monitorDataPointer *Universal)
{
    int bufferSize = *(Universal->bufferSizePtr); 
    int *fifo = *(Universal->fifoPtr); 
    monitorDir *mySubDir = *(Universal->mySubDirPtr); 
    char *Dir;
    void *receivedBuffer = malloc_hook(1);
    //while(read_hook(fifo[READ],&receivedBuffer,bufferSize)!=NULL){
    while(read_hook(fifo[0],&receivedBuffer,bufferSize)!=0){
        Dir = (char*) receivedBuffer;
        if(!mySubDir->Nsub_dirs)
            mySubDir->sub_dir = (char**) malloc_hook(sizeof(char*)*1);
        else
            mySubDir->sub_dir = (char**) realloc_hook(mySubDir->sub_dir, sizeof(char*)*((long unsigned int) (mySubDir->Nsub_dirs+1)));
        mySubDir->sub_dir[mySubDir->Nsub_dirs] = (char*) malloc_hook(strlen(Dir)+1);
        strcpy(mySubDir->sub_dir[mySubDir->Nsub_dirs], Dir);
        mySubDir->Nsub_dirs++;
        //printf("%d READ %s\n",getpid()%10, Dir);
        parseDir(Universal, Dir);
    }
    free(receivedBuffer);
}

void parse(monitorDataPointer *Universal, char *filename)
{
    int filter, index;
    char *input, **word;
    char *citizenID = NULL, *name = NULL, *surname = NULL, *country = NULL, *age = NULL, *virus = NULL, *vaccinated = NULL, *date = NULL;
    input = (char*) malloc(sizeof(char)*201); 
    word = (char**) malloc(sizeof(char*)*5);

    char **vacced = *(Universal->vaccedPtr); 
    Record **Registry = *(Universal->RegistryPtr);
    int *inputSize = Universal->inputSizePtr;

    FILE *citizenRecordsFile = fopen(filename,"r");
    Record *NodePointer;
    int duplicate = 0; 

    if(citizenRecordsFile==NULL){
        //printf("ERROR: %s did not open\n",filename);
        perror("file");
        exit(1);
    }

    while (fgets(input,200,citizenRecordsFile)!=NULL){
        //input[strcspn(input, "\n")] = 0;  //delete newline character from the string if it exists
        if(input[strlen(input)-1]=='\n') 
            input[strlen(input)-1]='\0';
        citizenID = strtok (input, " \t");
        name = strtok (NULL, " \t");
        surname = strtok (NULL, " \t");
        country = strtok (NULL, " \t");
        age = strtok (NULL, " \t");
        virus = strtok (NULL, " \t");
        vaccinated = strtok (NULL, " \t");
        date = strtok (NULL, " \t");

        if(vaccinated!=NULL)
        {
            if(!strcmp(vaccinated, "YES"))
                word[4]=vacced[1];
            else
                word[4]=vacced[0];
        }
        else
        {
            //printf("ERROR IN RECORD %s %s %s %s %s %s %s %s\n",citizenID, name, surname, country, age, virus, vaccinated, date!=NULL ? date : "");
            continue;
        }

        word[0]=virus;
        word[2]=country;
        if(virus_exists(Universal, &word, &filter, &index)==0)
            create_datatypes(Universal);

        duplicate = existsIn_hashtable(Registry[my_hash(atoi(citizenID))], &NodePointer, citizenID, virus);
        if(!strcmp(word[4], "NO") && date!=NULL){
            printf("ERROR (date) IN RECORD %s %s %s %s %s %s %s %s\n",citizenID, name, surname, country, age, virus, vaccinated, date);
            continue;
        }
        else if(!duplicate) //no duplicates
        {
            insertAt_hashtable(&Registry[my_hash(atoi(citizenID))], citizenID, name, surname, word[3], age, word[1], word[4], date);
            (*inputSize)++;
        }
        else if(duplicate){
            printf("ERROR (duplicate) IN RECORD %s %s %s %s %s %s %s %s \n", citizenID, name, surname, country, age, virus, vaccinated, date!=NULL ? date : "");
            continue;
        }
    }
    fclose(citizenRecordsFile);
    free(input);
    free(word);
}

void parseDir(monitorDataPointer* Universal, char *file)
{
    // monitorDir *mySubDir; //= *(Universal->mySubDirPtr); 
    DIR *passed_dir;
    struct dirent *passed_file;
    char *filename = NULL;
    struct stat filestatus; 
    char *dirname = *(Universal->dirnamePtr);    
    char *temp = (char*) malloc(sizeof(char)*(strlen(dirname)+strlen(file)+2));
    snprintf(temp, strlen(dirname)+strlen(file)+2, "%s/%s", dirname, file);
    passed_dir = opendir(temp);
    if (passed_dir == NULL){
        perror("opendir");
        exit(EXIT_FAILURE);
    }
    while((passed_file=readdir(passed_dir)) != NULL)
        if(strcmp(passed_file->d_name,".") && strcmp(passed_file->d_name,"..") && passed_file->d_ino!=0){
            filename = (char*) realloc(filename, strlen(dirname)+2*strlen(file)+10); 
            snprintf(filename, strlen(temp)+strlen(passed_file->d_name)+2 , "%s/%s", temp, passed_file->d_name);
            if(!isFileRead(Universal->mySubDirPtr, passed_file->d_name))
            {
                // printf("%s\n",passed_file->d_name);
                // mySubDir = *(Universal->mySubDirPtr); 
                if (stat(filename, &filestatus) == -1){
                    perror("stat");
                    exit(EXIT_FAILURE);
                }
                else
                    if(filestatus.st_size>1)
                        parse(Universal, filename);
            }
        }
    closedir(passed_dir);
    free(temp);
    free(filename);
}

void storeDataIntoDatatypes(monitorDataPointer* Universal)
{
    Record *NodePointer = NULL;
    Record **Registry = *(Universal->RegistryPtr);
    Sentinel **skiplist = *(Universal->skipListPtr);
    char **bloomfilter = *(Universal->bloomFilterPtr);
    char *citizenID = NULL/*, *name = NULL, *surname = NULL, *country = NULL, *age = NULL, *virus = NULL*/, *vaccinated = NULL;//, *date = NULL;
    char **vacced = *(Universal->vaccedPtr); 
    char **word = (char**) malloc(sizeof(char*)*4);
    char *bloomString = (char*) malloc(sizeof(char));
    int j=0, k=1, filter, index, first=1;
    int inputSize = *(Universal->inputSizePtr);
    if(inputSize>=2)
        k = abs(log2(inputSize)); 
    for(int i=0; i<*(Universal->NskipListsPtr); i++)
    {            
        skiplist[i]->NSkipLevels = k;
        skiplist[i]->next = (Node**) realloc(skiplist[i]->next, sizeof(Node*)*((long unsigned int) k));
        for(int l=0; l<k; l++)
            skiplist[i]->next[l] = NULL;
    } 

    for(int i=0; i<10; i++)
    {
        k=1;
        j=0;
        while(k)
        {
            k = exportFrom_hashtable(Registry[i], &NodePointer, &citizenID/*, &name, &surname, &country, &age*/, &word[0], &vaccinated/*, &date*/, j, first);
            if(k==0) break;
            if(first)
                first--;
            //if(NodePointer==NULL)
            //    continue;
            //printf("%d %s\n",in++,citizenID);
            word[2]=NULL;
            virus_exists(Universal, &word, &filter, &index); //is called to get the index of the bloomFilter of the current virus            

            if(vaccinated==vacced[1])
            {
                bloomString = (char*) realloc(bloomString, sizeof(char)*( strlen(citizenID) + strlen(word[1]) + 1 ) );
                snprintf(bloomString, strlen(citizenID)+strlen(word[1]) + 1, "%s%s", citizenID, word[1]);
                insertAt_bloomFilter( bloomfilter,  filter, *(Universal->bloomSizePtr), (unsigned char*) bloomString);
                //bloomCheck = isItemIn_bloomFilter( bloomfilter, filter, *(Universal->bloomSizePtr), (unsigned char*) bloomString);
                //printf("Check for '%s' in filter=%d: %d\n",citizenID,filter,bloomCheck);
                insertAt_skipList(&(skiplist[2*filter]), NodePointer);
                //printll(skiplist[2*filter]->next[0],0);
            }
            else if(vaccinated==vacced[0]){
                insertAt_skipList(&(skiplist[2*filter+1]), NodePointer);
                //printll(skiplist[2*filter+1]->next[0],0);
            }
            j++;
        }
    }
    free(word);
    free(bloomString);
    free(citizenID);
}

void sendBloomFilters(monitorDataPointer *Universal)
{
    int NbloomFilters = *(Universal->NbloomFiltersPtr); 
    int bufferSize = *(Universal->bufferSizePtr); 
    int bloomSize = *(Universal->bloomSizePtr); 
    char **bloomFilter = *(Universal->bloomFilterPtr); 
    char **virusData = *(Universal->virusDataPtr); 
    int *fifo = *(Universal->fifoPtr); 
    for(int i=0;i<NbloomFilters;i++)
    {
        // write_hook(fifo[WRITE], virusData[i], makeHeader(2, strlen(virusData[i])), bufferSize, strlen(virusData[i]));
        // write_hook(fifo[WRITE], bloomFilter[i], makeHeader(2, bloomSize), bufferSize, bloomSize);
        write_hook(fifo[1], virusData[i], makeHeader(2, strlen(virusData[i])+1), bufferSize, strlen(virusData[i])+1);
        write_hook(fifo[1], bloomFilter[i], makeHeader(2, bloomSize), bufferSize, bloomSize);
        //printf("%d) virus sent = %s with strlen(%d) from %d\n",i, virusData[i], strlen(virusData[i]), getpid());        
        //printf("(%d) virus sent = %s from %d\n\n",i, virusData[i], getpid());
    }
}

void sendReadyMsg(monitorDataPointer *Universal)
{
    int bufferSize = *(Universal->bufferSizePtr); 
    int *fifo = *(Universal->fifoPtr); 
    char *msg = (char*) malloc_hook(6);
    strcpy(msg,"READY"); 
    //write_hook(fifo[WRITE], msg, makeHeader(2, 6), bufferSize, 6);
    write_hook(fifo[1], msg, makeHeader(2, 6), bufferSize, 6);
    free(msg);
    //kill(getppid(),SIGUSR2);
}

void waitForParent(monitorDataPointer *Universal)
{
    int bufferSize = *(Universal->bufferSizePtr);
    int *fifo = *(Universal->fifoPtr);
    void *receivedBuffer = NULL;
    char **argument = malloc_hook(sizeof(char*)*4);
    while(1)
    {
        //kill(getpid(), SIGSTOP);
        // printf("SIGCONT\n");
        checkFlag(Universal);
        if(read_hook(fifo[READ],&receivedBuffer,bufferSize)!=0){
            argument[0] = strtok(receivedBuffer,"|");
            argument[1] = strtok(NULL,"|");   //CITIZENID
            argument[2] = strtok(NULL,"|");   //VIRUS
            argument[3] = strtok(NULL,"|");   //DATE
            if(!strcmp(argument[0],"REQUEST"))
                travelRequest(Universal, argument[1], argument[2], argument[3]);
            else if(!strcmp(argument[0],"SEARCH"))
                searchVaccinationStatus(Universal, argument[1]);
            else if(!strcmp(receivedBuffer, "EXIT")){
                free(receivedBuffer);
                exitNow(&Universal);
                break;
            }
            argument[0] = NULL;
            argument[1] = NULL;   //CITIZENID
            argument[2] = NULL;   //VIRUS
            argument[3] = NULL;   //DATE
        }
    }
    free(argument);
}

void travelRequest(monitorDataPointer *Universal, char *citizenID, char *virusName, char *date)
{
    Sentinel **skiplist = *(Universal->skipListPtr);
    int bufferSize = *(Universal->bufferSizePtr);
    int *fifo = *(Universal->fifoPtr);
    int filter = 0;
    char **word = (char**) malloc(sizeof(char*)*3);
    char *answer = NULL; 
    word[0]=virusName;
    word[2]=NULL;        

    if(virus_exists(Universal, &word, &filter, NULL))
    {
        word[0] = citizenID;
        word[1] = virusName; 
        if(search_skipList(&(skiplist[2*filter]->next[0]), word, (skiplist[2*filter])->NSkipLevels))
        {
            answer = (char*) malloc_hook(sizeof(char)*(strlen(word[2])+6));
            snprintf(answer, strlen(word[2])+6 , "YES|%s|", word[2]); 
            if(recentlyVacced(word[2], date)>0)
                addRequest(Universal, date, "NONE", NULL, 1, 0);
            else
                addRequest(Universal, date, "NONE", NULL, 0, 0);
            // printf("\x1b[32mVACCINATED ON %s\x1b[0m\n", word[2]);
        }
        else
        {
            answer = (char*) malloc_hook(sizeof(char)*(3));
            snprintf(answer, 4 , "NO|");            
            addRequest(Universal, date, "NONE", NULL, 0, 1);  
            // printf("\x1b[31mNOT VACCINATED\x1b[0m\n");         
        }
        write_hook(fifo[WRITE], answer, makeHeader(2, strlen(answer)+1), bufferSize, strlen(answer)+1);
        free(answer);
    }
    else
    {
        printf("Skip List for the virus %s is non-existent.\n",virusName);
    }
    free(word);
}

void searchVaccinationStatus(monitorDataPointer *Universal, char *citizenID)
{
    int bufferSize = *(Universal->bufferSizePtr);
    int *fifo = *(Universal->fifoPtr);
    int NskipLists = *(Universal->NskipListsPtr);
    char **virusData = *(Universal->virusDataPtr);
    Sentinel **skiplist = *(Universal->skipListPtr);
    Record **Registry = *(Universal->RegistryPtr);
    Record *NodePointer = NULL;
    char **word = (char**) malloc(sizeof(char*)*3);
    // char *answer = NULL;
    char **data = (char**) malloc_hook(sizeof(char*)*((long unsigned int) (NskipLists+2)));
    Header **head = (Header**) malloc_hook( sizeof(Header*)*((long unsigned int)(NskipLists+2)));
    int *size = (int*) malloc_hook(sizeof(int)*((long unsigned int) NskipLists+2));
    for(unsigned int i=0; i<((unsigned int) (NskipLists+2));i++){
        data[i] = NULL;
        head[i] = NULL;
        size[i] = 0;
    }
    if(search_hashtable(Registry[my_hash(atoi(citizenID))],  &NodePointer, citizenID))
    {
        // answer = (char*) malloc_hook(sizeof(char)*(strlen(NodePointer->citizenID)+1+strlen(NodePointer->name)+1+strlen(NodePointer->surname)+1+strlen(NodePointer->country)+2));
        // snprintf(answer, strlen(NodePointer->citizenID)+1+strlen(NodePointer->name)+1+strlen(NodePointer->surname)+1+strlen(NodePointer->country)+2 , "%s|%s|%s|%s|", NodePointer->citizenID, NodePointer->name, NodePointer->surname, NodePointer->country);
        // write_hook(fifo[WRITE], answer, makeHeader(2, strlen(answer)+1), bufferSize, strlen(answer)+1);
        // free(answer);
        // answer = (char*) malloc_hook(sizeof(char)*(4+strlen(NodePointer->age)+2));
        // snprintf(answer, 4+strlen(NodePointer->age)+2 , "AGE|%s|", NodePointer->age);
        // write_hook(fifo[WRITE], answer, makeHeader(2, strlen(answer)+1), bufferSize, strlen(answer)+1);
        // free(answer);

        // data[0] = (char*) malloc_hook(sizeof(char)*(strlen(NodePointer->citizenID)+1+strlen(NodePointer->name)+1+strlen(NodePointer->surname)+1+strlen(NodePointer->country)+2));
        // snprintf(data[0], strlen(NodePointer->citizenID)+1+strlen(NodePointer->name)+1+strlen(NodePointer->surname)+1+strlen(NodePointer->country)+2 , "%s|%s|%s|%s|", NodePointer->citizenID, NodePointer->name, NodePointer->surname, NodePointer->country);
        data[0] = (char*) malloc_hook(sizeof(char)*(strlen(NodePointer->citizenID)+1+strlen(NodePointer->name)+1+strlen(NodePointer->surname)+1+strlen(NodePointer->country)+2));
        snprintf(data[0], strlen(NodePointer->citizenID)+1+strlen(NodePointer->name)+1+strlen(NodePointer->surname)+1+strlen(NodePointer->country)+2 , "%s %s %s %s ", NodePointer->citizenID, NodePointer->name, NodePointer->surname, NodePointer->country);
        
        head[0] = makeHeader(2, strlen(data[0])+1);
        size[0] = strlen(data[0])+1;

        data[1] = (char*) malloc_hook(sizeof(char)*(4+strlen(NodePointer->age)+2));
        snprintf(data[1], 4+strlen(NodePointer->age)+2 , "AGE %s ", NodePointer->age);
        head[1] = makeHeader(2, strlen(data[1])+1);
        size[1] = strlen(data[1])+1;
        for(int i=0; i<NskipLists; i++)
        {
            word[0] = citizenID;
            if(i%2==0)
            {
                word[1] = virusData[i/2];
                if(search_skipList(&(skiplist[i]->next[0]), word, (skiplist[i])->NSkipLevels))
                {
                    // printf("\x1b[32m%s YES %s\x1b[0m\n", virusData[i/2], word[2]);
                    // answer = (char*) malloc_hook(sizeof(char)*(strlen(virusData[i/2])+strlen(word[2])+7));
                    // snprintf(answer, strlen(virusData[i/2])+strlen(word[2])+7 , "%s|YES|%s|", virusData[i/2], word[2]);
                    // write_hook(fifo[WRITE], answer, makeHeader(2, strlen(answer)+1), bufferSize, strlen(answer)+1);
                    // free(answer);
                    data[i] = (char*) malloc_hook(sizeof(char)*(strlen(virusData[i/2])+strlen(word[2])+16));
                    snprintf(data[i], strlen(virusData[i/2])+strlen(word[2])+16 , "%s VACCINATED ON %s", virusData[i/2], word[2]);
                    head[i] = makeHeader(2, strlen(data[i])+1);
                    size[i] = strlen(data[i])+1;
                }
            }
            else if(i%2==1)
            {
                word[1] = virusData[(i-1)/2];
                if(search_skipList(&(skiplist[i]->next[0]), word, (skiplist[i])->NSkipLevels))
                {
                    // printf("\x1b[32m%s YES %s\x1b[0m\n", virusData[(i-1)/2]);
                    // answer = (char*) malloc_hook(sizeof(char)*(strlen(virusData[(i-1)/2])+5));
                    // snprintf(answer, strlen(virusData[(i-1)/2])+5 , "%s|NO|", virusData[(i-1)/2]); 
                    // write_hook(fifo[WRITE], answer, makeHeader(2, strlen(answer)+1), bufferSize, strlen(answer)+1);
                    // free(answer);
                    data[i] = (char*) malloc_hook(sizeof(char)*(strlen(virusData[(i-1)/2])+20));
                    snprintf(data[i],  strlen(virusData[(i-1)/2])+20 , "%s NOT YET VACCINATED", virusData[(i-1)/2]); 
                    head[i] = makeHeader(2, strlen(data[i])+1);
                    size[i] = strlen(data[i])+1;
                }  
            }
        }
        multiple_write_hook(fifo[WRITE], data, head, NskipLists, bufferSize, size);
    }
    free(word);
    free(data);
    free(head);
    free(size);
}

void exitNow(monitorDataPointer **Universal)
{
    monitorDataPointer *temp = *Universal;
    Sentinel **skiplist = *(temp->skipListPtr);
    Record** Registry = *(temp->RegistryPtr);
    monitorDir *mySubDir = *(temp->mySubDirPtr);
    char **virusData = *(temp->virusDataPtr);
    char **countryData = *(temp->countryDataPtr);
    char **vacced = *(temp->vaccedPtr);
    char **bloomfilter = *(temp->bloomFilterPtr);
    char *filename = *(temp->dirnamePtr);
    int *fifo = *(temp->fifoPtr);
    //free hashTable
    for(int i=0; i<10; i++)
		free_hashtable(Registry[i]);
    free(Registry);
    //free skipList
    for(int i=0; i<*(temp->NskipListsPtr); i++)
        free_skipList(&(skiplist[i]));
    free(skiplist);
    //free bloomFilter
    free_bloomFilter(&bloomfilter, *(temp->NbloomFiltersPtr));
    //free countryData array
    for(int i=0; i<*(temp->NcountriesPtr); i++)
        free(countryData[i]);
    free(countryData);
    //free virusData array
    for(int i=0; i<*(temp->NbloomFiltersPtr); i++)
        free(virusData[i]);
    free(virusData);
    //free vacced array
    for(int i=0; i<2; i++)
        free(vacced[i]);
    free(vacced);
    //
    free(fifo);
    //
    for(int i=0;i<mySubDir->Nread_files;i++)
        free(mySubDir->read_files[i]);
    for(int i=0;i<mySubDir->Nsub_dirs;i++)
        free(mySubDir->sub_dir[i]);
    free(mySubDir);
    //free filename string
    free(filename);
    //free the Universal Datapointer 
    free(*Universal);
    printf("\033[1;37mSuccessfuly cleaned monitor %d data.\n",getpid()%10);
}