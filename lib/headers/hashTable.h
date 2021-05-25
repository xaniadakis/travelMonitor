#include <stdio.h>

#ifndef hashTableStructHeader
#define hashTableStructHeader
#include "../structures/hashTableStruct.h"
#endif

Record* insertAt_hashtable(Record**, char*, char*, char*, char*, char*, char*, char*, char*);
int exportFrom_hashtable(Record*, Record**, char**/*, char**, char**, char**, char***/, char**, char**/*, char** */, int, int);
void alter_hashNode(Record*, char*, char*);
int existsIn_hashtable(Record*, Record**, char*, char*);
int search_hashtable(Record*,  Record**, char*);
void print(Record*);
void free_hashtable(Record*);