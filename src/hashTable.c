#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef hashFunctionsHeader
#define hashFunctionsHeader
#include "../lib/headers/hashFunctions.h"
#endif

#ifndef hashTableStructHeader
#define hashTableStructHeader
#include "../lib/structures/hashTableStruct.h"
#endif

void print(Record* temp)
{
    if (temp!=NULL)
        printf("printing table[%d]\n",my_hash(atoi(temp->citizenID)));

    while (temp != NULL)
    {
        printf(" %s %s %s %s %s %s %s", temp->citizenID,  temp->name, temp->surname, temp->country, temp->age, temp->virus, temp->vaccinated);
        if(!strcmp(temp->vaccinated, "YES"))
            printf(" %s \n", temp->date);
        else 
            printf("\n");
        temp = temp->next;
    }
}

Record* insertAt_hashtable(Record** Registry, char *citizenID, char *name, char *surname, char *country, char *age, char *virus, char *vaccinated, char *date)
{
    Record *temp = *Registry;//, *prev;
    Record *new = (Record*) malloc(sizeof(Record));
    new->citizenID=(char*) malloc(sizeof(char)*(strlen(citizenID)+1)); 
    strcpy(new->citizenID, citizenID);
    new->name=(char*) malloc(sizeof(char)*(strlen(name)+1)); 
    strcpy(new->name, name);
    new->surname=(char*) malloc(sizeof(char)*(strlen(surname)+1)); 
    strcpy(new->surname, surname);
    new->country=country;  //points at countryData array
    new->age=(char*) malloc(sizeof(char)*(strlen(age)+1)); 
    strcpy(new->age, age);
    new->virus=virus;    //points at virusData array
    new->vaccinated=vaccinated;     //points at vacced array
    if(!strcmp(vaccinated, "YES"))
    {
        new->date=(char*) malloc(sizeof(char)*(strlen(date)+1)); 
        strcpy(new->date, date);
    }    

    new->next = NULL;

    if((*Registry)==NULL)
        (*Registry) = new;
    else{
        while(temp->next!=NULL)
            temp = temp->next; 
        temp->next = new;  
    }
    return new;
}

int exportFrom_hashtable(Record* Registry, Record** NodePointer, char **citizenID/*, char **name, char **surname, char **country, char **age*/, 
                         char **virus, char **vaccinated,/* char **date,*/ int counter, int first)
{   
    Record *temp;
    temp = Registry;
    int i=0, found=0;
    if(Registry == NULL){
        return 0;
    }
    else
    {
        while(temp != NULL){
            if(i==counter)
            {
                found=1;
                break;
            }
            temp = temp->next;
            i++;
        }
    }
    if(found==0)
        return 0;
    *NodePointer = temp;
    if(first==1){
        *citizenID=(char*) malloc(sizeof(char)*(strlen(temp->citizenID)+1));
    }
    else{
        *citizenID=(char*) realloc((*citizenID), sizeof(char)*(strlen(temp->citizenID)+1)); 
    }
    strcpy(*citizenID, temp->citizenID);
    *virus=temp->virus;         //we dont malloc and strcpy because this points to virusData array
    *vaccinated=temp->vaccinated;           //we dont malloc and strcpy because this points to vacced array
    return 1;
}

void alter_hashNode(Record* NodePointer, char* vaccinated, char* date)
{
    if(NodePointer!=NULL)
        if(!strcmp((NodePointer)->vaccinated, "NO") && !strcmp(vaccinated, "YES"))
        {
            (NodePointer)->vaccinated = vaccinated;
            (NodePointer)->date = (char*) malloc(sizeof(char)*(strlen(date)+1)); 
            strcpy((NodePointer)->date, date);
        }
}

int existsIn_hashtable(Record* Registry,  Record** NodePointer, char *citizenID, char *virus)
{
    Record *temp = Registry;
    while(temp!=NULL)
    {
        //printf("%s ?= %s and %s ?= %s\n",temp->citizenID,citizenID,temp->virus, virus);
        if((strcmp(temp->citizenID, citizenID)==0)&&(strcmp(temp->virus, virus)==0)){
            *NodePointer=temp;
            return 1;
        }
        temp = temp->next;
    }
    return 0;
}

int search_hashtable(Record* Registry,  Record** NodePointer, char *citizenID)
{
    Record *temp = Registry;
    while(temp!=NULL)
    {
        //printf("%s ?= %s and %s ?= %s\n",temp->citizenID,citizenID,temp->virus, virus);
        if(!strcmp(temp->citizenID, citizenID)){
            *NodePointer=temp;
            return 1;
        }
        temp = temp->next;
    }
    return 0;
}

void free_hashtable(Record *Registry)
    {
    Record* temp;
    while (Registry!=NULL)
    {
        temp = Registry;
        Registry = Registry->next;
        free(temp->citizenID);
        free(temp->name);
        free(temp->surname);
        free(temp->age);
        if(!strcmp(temp->vaccinated,"YES"))
            free(temp->date);
        free(temp);
    }
}
