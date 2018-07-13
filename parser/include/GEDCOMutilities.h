#ifndef GEDCOMUTILITIES_H
#define GEDCOMUTILITIES_H

#include "GEDCOMparser.h"
#include "LinkedListAPI.h"

//struct to temporarily hold tag and associated individual
typedef struct{
    char tag[26];
    Individual* temp;
} tagIndi; 

typedef struct{
    int num;
    Individual* temp;
} storeIndi; 

typedef struct{
    int num;
    Family* temp;
} storeFam;

/** Function to create submitter recors
 *@return a pointer to the generated submitter record
 *@param char filename
 *@param gedcomerror to report any errors
 *@param token/ should be submitter tag
 **/
Submitter* createSubmitter(char* fileName, GEDCOMerror* temperror, char* token);

/** Function to check for CONT CONC tags
 *@param char filename
 *@param gedcom file pointer
 *@param current line number
 **/
void contconcCheck(char* line, FILE* inFile, int* lineNumb, GEDCOMerror* error);

/** Function to copy an indvidual
 *@return new memory associated with individual
 *@param individual to copy
 **/
Individual* copyIndi(Individual* toCopy);

/** Function to copy an event
 *@return new memory associated with event
 *@param event to copy
 **/
Event* copyEvent(Event* toCopy);

/** Function to copy a field
 *@return new memory associated with field
 *@param field to copy
 **/
Field* copyField(Field* toCopy);

/** Dummy delete function to intialize list
 *@param dummy parameter
 **/
void dummyDelete(void *data);

/** Function to print an individual
 *@return string pointer to individuals name
 *@param individual whose name to print
 **/
char* printName(Individual* toBePrinted);

/** Compare two indviduals events
 *@return bool dependant on events similarity
 *@param first individual to compare
 *@param second individual to compare
 **/
bool cmpEvent(Individual* a, const Individual* b);

/** Function to create families for GEDCOM object
 *@param GEDCOM object
 *@param GEDCOM filename
 *@param list of individuals with associated tags
 *@param GEDCOMerror to return errors if need be
 **/
void createFamilies (GEDCOMobject* temp, char* fileName, List tempStore, GEDCOMerror* error);

/** Custom fgets to incorporate GEDCOM standads
 *@return true if sucessfully retrieved GEDCOM line false otherwise
 *@param line to read into from file
 *@param length of line to read, should be 256
 *@param FILE pointer to GEDCOM file
 **/
bool customFgets(char* line, int len, FILE* inFile, GEDCOMerror* error);

/** Compare tags of two individuals
 *@return bool dependant on strcmp of two tags
 *@param list of individuals with associated tags
 *@param GEDCOMerror to return errors if need be
 **/
bool compareTag(const void* a,const void* b);

/** delete function for temp individual struct
 *@param tagIndi struct to delete
 **/
void destroyNodeData(void *data);

/** Function to tokenize a line of GEDCOM to get value
 *@return pointer to value part of line
 *@param line to tokenize
 **/
char* tokenize (char line[]);

bool findTag(const void* first,const void* second);

bool findFamily(const void* a,const void* b);

bool findIndividual(const void* a,const void* b);

void getChildren(List *descendants, const Individual *individual, unsigned int maxGen, int count);

void recursiveDescendant(List *descendants, Family *family, unsigned int maxGen, int count);

void recursiveAnscestor(List *descendants, Family *family, unsigned int maxGen, int count);

int compareSurname(const void* a,const void* b);

bool findIndi(const void* a,const void* b);

char* GEDCOMtoJSON(char* fileName);

char* createIndJSON(char* fileName);

char* filterfiles(char* fileName);

void JSONaddindi(char* fileName, char* firstname, char* lastname);


#endif
