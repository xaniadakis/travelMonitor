#include <stdio.h>

#ifndef DataStructHeader
#define DataStructHeader
#include "../structures/DataStruct.h"
#endif

#ifndef skipStructHeader
#define skipStructHeader
#include "../structures/skipStruct.h"
#endif

void uploadMonitorData(monitorDataPointer **, Record ***, Sentinel ***, char ***, char ***,
                       char ***, char ***, char **, int *, int *, int *, 
                       int *, int *, int *, monitorDir **, int **, reqReg**, char**); 
void receiveInfo(monitorDataPointer*);
void receiveSubdirs(monitorDataPointer*);                         
void parse(monitorDataPointer*, char*);
void parseDir(monitorDataPointer*, char*);
void storeDataIntoDatatypes(monitorDataPointer*);
void sendBloomFilters(monitorDataPointer*);
void sendReadyMsg(monitorDataPointer*);
void waitForParent(monitorDataPointer*);
void travelRequest(monitorDataPointer*, char*, char*, char*);
void searchVaccinationStatus(monitorDataPointer*, char*);
void exitNow(monitorDataPointer**);
