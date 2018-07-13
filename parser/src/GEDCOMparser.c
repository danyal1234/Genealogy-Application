#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "LinkedListAPI.h"
#include "GEDCOMparser.h"
#include "GEDCOMutilities.h"

//***************************************** GEDCOOM object functions *****************************************

/** Function to create a GEDCOM object based on the contents of an GEDCOM file.
 *@pre File name cannot be an empty string or NULL.  File name must have the .ics extension.
 File represented by this name must exist and must be readable.
 *@post Either:
 A valid GEDCOM has been created, its address was stored in the variable obj, and OK was returned
 or
 An error occurred, the GEDCOM was not created, all temporary memory was freed, obj was set to NULL, and the
 appropriate error code was returned
 *@return the error code indicating success or the error encountered when parsing the GEDCOM
 *@param fileName - a string containing the name of the GEDCOM file
 *@param a double pointer to a GEDCOMobject struct that needs to be allocated
 **/
GEDCOMerror createGEDCOM(char* fileName, GEDCOMobject** obj){

    GEDCOMerror error;
    error.line = -1;
    //check if filename exists
    if(fileName == NULL){
        error.type = INV_FILE;
        return error;
    }

    FILE* inFile = fopen(fileName, "r");
    char *token;
    char *tag;
    char submTag[32];
    int submCheck = 0;
    int lineNumb = 1;
    int charCheck = 0;
    char* line = malloc(sizeof(char)*256);
    List tempStore = initializeList(&printIndividual, &destroyNodeData, &compareIndividuals);

    //check if file was opened properly
    if(inFile == NULL){
        error.type = INV_FILE;
        free(inFile);
        free(line);
        return error;
    }

    char* tempFile = malloc(sizeof(char) * (strlen(fileName) + 1));
    strcpy(tempFile, fileName);

    token = strtok(tempFile, ".");
    if(token != NULL) {
        token = strtok(NULL, ".");
    }

    //validate file tag
    if(token == NULL || strcmp(token, "ged") != 0) {
        free(tempFile);
        fclose(inFile);
        free(line);
        error.type = INV_FILE;
        return error;
    }

    free(tempFile);

    //check if the file is readable
    if(!customFgets(line, 256, inFile, &error)) {
        error.type = INV_FILE;
        clearList(&tempStore);
        free(line);
        fclose(inFile);
        return error;
    }
    char tempFirst[26];
    strcpy(tempFirst, line);
    token = strtok(tempFirst, " ");
    int headNum = atoi(token);
    token = strtok(NULL, " ");
    if(headNum != 0 && strncmp(token,"HEAD",4) == 0){
        error.type = INV_HEADER;
        clearList(&tempStore);
        free(line);
        fclose(inFile);
        return error;
    }
    //validate header first line
    else if(strncmp(line,"0 HEAD", 6)!=0){
        error.type = INV_GEDCOM;
        clearList(&tempStore);
        free(line);
        fclose(inFile);
        return error;
    }

    //create required fields to start parsing
    Header* header = malloc(sizeof(Header));
    header->otherFields = initializeList(&printField, &deleteField, &compareFields);
    GEDCOMobject* temp = malloc(sizeof(GEDCOMobject));
    temp->individuals = initializeList(&printIndividual, &deleteIndividual, &compareIndividuals);
    temp->families = initializeList(&printFamily, &deleteFamily, &compareFamilies);

    while(1){

        customFgets(line, 256, inFile, &error);
        lineNumb++;
        if(error.type != OK){
            //deleteGEDCOM(temp);
            clearList(&tempStore);
            free(line);
            fclose(inFile);
            error.type = INV_HEADER;
            error.line = lineNumb;
            return error;
        }
        contconcCheck(line, inFile, &lineNumb, &error);

        //get GECOM file source
        if(strncmp(line,"1 SOUR ", 6)==0){
            token = tokenize(line);
            if(token == NULL){
                deleteGEDCOM(temp);
                free(line);
                clearList(&header->otherFields);
                free(header);
                fclose(inFile);
                error.type = INV_HEADER;
                error.line = lineNumb;
                return error;
            }
            sprintf(header->source, "%s", token);
        }
        //parse GEDCOM version
        else if(strncmp(line,"1 GEDC", 6)==0){
            customFgets(line, 256, inFile, &error);
            lineNumb++;
            if(error.type != OK){
                clearList(&tempStore);
                free(line);
                fclose(inFile);
                error.type = INV_HEADER;
                error.line = lineNumb;
                return error;
            }
            contconcCheck(line, inFile, &lineNumb, &error);
            if(strncmp(line,"2 VERS", 6)==0){
                token = tokenize(line);
                if(token == NULL){
                    free(line);
                    clearList(&header->otherFields);
                    free(header);
                    fclose(inFile);
                    error.type = INV_HEADER;
                    error.line = lineNumb;
                    return error;
                }
                header->gedcVersion = atof(token);
            }
            else if(strstr(line,"VERS") != NULL){
                clearList(&tempStore);
                free(line);
                fclose(inFile);
                error.type = INV_RECORD;
                error.line = lineNumb;
                return error;
            }
        }
        //parse char type of GEDCOM document
        else if(strncmp(line,"1 CHAR",6) == 0){
            charCheck = 1;
            token = tokenize(line);
            if(token == NULL){
                clearList(&tempStore);
                free(line);
                fclose(inFile);
                error.type = INV_HEADER;
                error.line = lineNumb;
                return error;
            }
            if(strncmp(token,"ANSEL", 5) == 0){
                header->encoding = ANSEL;
            }
            else if(strncmp(token,"UTF-8", 5) == 0){
                header->encoding = UTF8;
            }
             else if(strncmp(token,"UNICODE", 7) == 0){
                header->encoding = UNICODE;
            }
            else if(strncmp(token,"ASCII", 5) == 0){
                header->encoding = ASCII;
            }
            else{
                clearList(&tempStore);
                free(line);
                fclose(inFile);
                error.type = INV_HEADER;
                error.line = lineNumb;
                return error;
            }

        }
        //check if submitter present
        else if(strncmp(line,"1 SUBM",6) == 0){
            token = tokenize(line);
            submCheck = 1;
            if(token == NULL){
                clearList(&tempStore);
                free(line);
                fclose(inFile);
                error.type = INV_HEADER;
                error.line = lineNumb;
                error.type = INV_HEADER;
                error.line = lineNumb;
                return error;
            }
            sprintf(submTag, "%s", token);
        }
        //break if header file over
        else if(line[0] == '0'){
            break;
        }
        else{
            //insert header field if correct field
            Field* field = malloc(sizeof(Field));
            token = strtok(line, " ");
            token = strtok(NULL, " ");
            if(token == NULL){
                clearList(&tempStore);
                free(line);
                fclose(inFile);
                free(field);
                error.type = INV_HEADER;
                error.line = lineNumb;
                return error;
            }
            field->tag = malloc(sizeof(char)* (strlen(token) + 1));
            sprintf(field->tag, "%s", token);
            token = strtok(NULL, "");
            if(token == NULL){
                clearList(&tempStore);
                free(line);
                fclose(inFile);
                error.type = INV_HEADER;
                error.line = lineNumb;
                return error;
            }
            field->value = malloc(sizeof(char)* (strlen(token) + 1));
            sprintf(field->value, "%s", token);
            insertBack(&header->otherFields,field);
        }

    }


    //validate header fields
    if(strlen(header->source) == 0 || header->gedcVersion == 0 || charCheck != 1 || submCheck == 0){
        //deleteGEDCOM(temp);
        clearList(&tempStore);
        free(line);
        fclose(inFile);
        error.type = INV_HEADER;
        error.line = lineNumb;
        return error;
    }

    temp->header = header;
    GEDCOMerror* temperror = malloc(sizeof(GEDCOMerror));
    Submitter* submitter = createSubmitter(fileName, temperror, submTag);
    if(temperror->type != OK){
        //deleteGEDCOM(temp);
        clearList(&tempStore);
        free(line);
        fclose(inFile);
        return *temperror;
    }
    free(temperror);
    header->submitter = submitter;
    temp->submitter = submitter;
    
   while(1){

        if(line[0] == '0'){
            //check if trailer and end parsing
            if(strncmp(line,"0 TRLR", 6) == 0){
                break;
            }
            //check if end of file
            else if(feof(inFile)){
                deleteGEDCOM(temp);
                clearList(&tempStore);
                free(line);
                fclose(inFile);
                error.type = INV_GEDCOM;
                return error;
            }
            token = strtok(line, " ");
            tag = strtok(NULL, " ");
            token = strtok(NULL, "");
            if(token== NULL){
                return error;
            }
            if(strncmp(token,"INDI", 4) == 0){
                Individual* indi = malloc(sizeof(Individual));
                char tempTag[10];
                strcpy(tempTag, tag);
                indi->events = initializeList(&printEvent, &deleteEvent, &compareEvents);
                indi->otherFields = initializeList(&printField, &deleteField, &compareFields);
                indi->families = initializeList(&printFamily, &dummyDelete, &compareFamilies);
                customFgets(line, 256, inFile, &error);
                lineNumb++;
                if(error.type != OK){
                    deleteGEDCOM(temp);
                    clearList(&tempStore);
                    free(line);
                    free(indi);
                    fclose(inFile);
                    error.type = INV_RECORD;
                    error.line = lineNumb;
                    return error;
                }
                contconcCheck(line, inFile, &lineNumb, &error);
                while(1){
                    if(strncmp(line,"1 NAME", 6) == 0){
                        token = strtok(line, " ");
                        token = strtok(NULL, " ");
                        token = strtok(NULL, " /");
                        if(token == NULL){
                            indi->givenName = malloc(sizeof(char));
                            strcpy(indi->givenName,"");
                        }
                        else{
                            indi->givenName = malloc(sizeof(char)* (strlen(token) + 1));
                            sprintf(indi->givenName, "%s", token);
                        }
                        token = strtok(NULL, " /");
                        if(token == NULL){
                            indi->surname = malloc(sizeof(char));
                            strcpy(indi->surname,"");
                        }
                        else{
                            indi->surname = malloc(sizeof(char)* (strlen(token) + 1));
                            sprintf(indi->surname, "%s", token);
                        }
                        customFgets(line, 256, inFile, &error);
                        lineNumb++;
                        if(error.type != OK){
                            deleteGEDCOM(temp);
                            clearList(&tempStore);
                            free(line);
                            fclose(inFile);
                            error.type = INV_RECORD;
                            error.line = lineNumb;
                            return error;
                        }
                        contconcCheck(line, inFile, &lineNumb, &error);
                        continue;
                    }

                    if(line[0] == '0'){
                        tagIndi* tempindi = malloc(sizeof(tagIndi));
                        strncpy(tempindi->tag,tempTag, 25);
                        tempindi->temp = indi;
                        insertBack(&tempStore,tempindi);
                        insertBack(&temp->individuals, indi);
                        break;
                    }

                    customFgets(line, 256, inFile, &error);
                        lineNumb++;
                        if(error.type != OK){
                            deleteGEDCOM(temp);
                            clearList(&tempStore);
                            free(line);
                            fclose(inFile);
                            error.type = INV_RECORD;
                            error.line = lineNumb;
                            return error;
                        }
                        contconcCheck(line, inFile, &lineNumb, &error);
                        continue;

                    /*token = strtok(line, " ");
                    token = strtok(NULL, " ");

                    if(token == NULL){
                        deleteGEDCOM(temp);
                        clearList(&tempStore);
                        free(line);
                        fclose(inFile);
                        error.type = INV_RECORD;
                        error.line = lineNumb;
                        return error;
                    }
                    
                    //check for individual event
                   if(strncmp(token,"ADOP",4) == 0 || strncmp(token,"BIRT",4) == 0 || strncmp(token,"BAPM",4) == 0 || strncmp(token,"BARM",4) == 0 ||
                        strncmp(token,"BASM",4) == 0 || strncmp(token,"BLES",4) == 0  || strncmp(token,"BURI",4) == 0  || strncmp(token,"CENS",4) == 0  
                        || strncmp(token,"CHR",3) == 0 || strncmp(token,"CHRA",4) == 0 || strncmp(token,"CONF",4) == 0 || strncmp(token,"CREM",4) == 0 
                        || strncmp(token,"DEAT",4) ==  0 || strncmp(token,"EMIG",4) == 0|| strncmp(token,"FCOM",4) == 0 || strncmp(token,"GRAD",4) == 0
                        || strncmp(token,"IMMI",4) == 0 || strncmp(token,"NATU",4) == 0 || strncmp(token,"ORDN",4) == 0 || strncmp(token,"RETI",4) == 0 
                        || strncmp(token,"PROB",4) == 0 || strncmp(token,"WILL",4) == 0 || strncmp(token,"EVEN",4) == 0 ){

                        Event* event = malloc(sizeof(Event));
                        event->otherFields = initializeList(&printField, &deleteField, &compareFields);
                        strncpy(event->type, token, 4);
                        while(1){
                            customFgets(line, 256, inFile, &error);
                            lineNumb++;
                            if(error.type != OK){
                                deleteGEDCOM(temp);
                                clearList(&tempStore);
                                free(line);
                                clearList(&event->otherFields);
                                free(event);
                                fclose(inFile);
                                error.type = INV_RECORD;
                                error.line = lineNumb;
                                return error;
                            }
                            contconcCheck(line, inFile, &lineNumb, &error);
                            //if event record over break loop
                            if(line[0] == '0' || line[0] == '1'){
                                if(event->date == NULL){
                                    event->date = malloc(sizeof(char));
                                    strcpy(event->date,"");
                                }
                                if(event->place == NULL){
                                    event->place = malloc(sizeof(char));
                                    strcpy(event->place,"");
                                }
                                break;
                            }
                            //retrieve event date
                            if(strncmp(line,"2 DATE",6) == 0){
                                token = tokenize(line);
                                if(token == NULL){
                                    deleteGEDCOM(temp);
                                    clearList(&tempStore);
                                    free(line);
                                    fclose(inFile);
                                    if(event->place == NULL){
                                        free(event->place);
                                    }
                                    free(event);
                                    error.line = lineNumb;
                                    error.type = INV_RECORD;
                                    return error;
                                }
                                event->date = malloc(sizeof(char)* (strlen(token) + 1));
                                sprintf(event->date, "%s", token);
                            }
                            //retrieve event place
                            else if(strncmp(line,"2 PLAC",6) == 0){
                                token = tokenize(line);
                                if(token == NULL){
                                    deleteGEDCOM(temp);
                                    clearList(&tempStore);
                                    free(line);
                                    fclose(inFile);
                                    if(event->date == NULL){
                                        free(event->date);
                                    }
                                    free(event);
                                    error.line = lineNumb;
                                    error.type = INV_RECORD;
                                    return error;
                                }
                                event->place = malloc(sizeof(char)* (strlen(token) + 1));
                                sprintf(event->place, "%s", token);
                            }
                            else{
                                //create appropariate event field if valid
                                Field* field = malloc(sizeof(Field));
                                token = strtok(line, " ");
                                token = strtok(NULL, " ");
                                if(token == NULL){
                                    deleteGEDCOM(temp);
                                    clearList(&tempStore);
                                    free(line);
                                    fclose(inFile);
                                    free(field);
                                    error.line = lineNumb;
                                    error.type = INV_RECORD;
                                    continue;
                                }
                                field->tag = malloc(sizeof(char)* (strlen(token) + 1));
                                sprintf(field->tag, "%s", token);
                                token = strtok(NULL, "");
                                if(token == NULL){
                                    free(field->tag);
                                    free(field);
                                    deleteGEDCOM(temp);
                                    clearList(&tempStore);
                                    free(line);
                                    fclose(inFile);
                                    error.line = lineNumb;
                                    error.type = INV_RECORD;
                                    continue;
                                }
                                field->value = malloc(sizeof(char)* (strlen(token) + 1));
                                sprintf(field->value, "%s", token);
                                insertBack(&event->otherFields,field);
                            }
                        }

                        insertBack(&indi->events, event);
                    }
                    else if(strncmp(tag,"FAMS",4) == 0 || strncmp(tag,"FAMC",4) == 0){
                            customFgets(line, 256, inFile, &error);
                            lineNumb++;
                            if(error.type != OK){
                                deleteGEDCOM(temp);
                                clearList(&tempStore);
                                free(line);
                                fclose(inFile);
                                error.type = INV_RECORD;
                                error.line = lineNumb;
                                return error;
                            }
                            contconcCheck(line, inFile, &lineNumb, &error);
                            continue;
                    }
                    else{
                        //create appropariate individual field if valid
                        Field* field = malloc(sizeof(Field));
                        if(tag == NULL){
                            deleteGEDCOM(temp);
                            clearList(&tempStore);
                            free(line);
                            fclose(inFile);
                            free(field);
                            error.line  = lineNumb;
                            error.type = INV_RECORD;
                            return error;
                        }
                        field->tag = malloc(sizeof(char)* (strlen(token) + 1));
                        sprintf(field->tag, "%s", tag);
                        token = strtok(NULL, "");
                        if(token == NULL){
                            free(field->tag);
                            free(field);
                            deleteGEDCOM(temp);
                            clearList(&tempStore);
                            free(line);
                            fclose(inFile);
                            error.line  = lineNumb;
                            error.type = INV_RECORD;
                            return error;
                        }
                        field->value = malloc(sizeof(char)* (strlen(token) + 1));
                        sprintf(field->value, "%s", token);
                        insertBack(&indi->otherFields,field);
                        customFgets(line, 256, inFile, &error);
                        lineNumb++;
                        if(error.type != OK){
                            deleteGEDCOM(temp);
                            clearList(&tempStore);
                            free(line);
                            fclose(inFile);
                            error.type = INV_RECORD;
                            error.line = lineNumb;
                            return error;
                        }
                        contconcCheck(line, inFile, &lineNumb, &error);
                    }*/
                }
            }
            else{
                customFgets(line, 256, inFile, &error);
                lineNumb++;
                if(error.type != OK){
                    deleteGEDCOM(temp);
                    clearList(&tempStore);
                    free(line);
                    fclose(inFile);
                    error.type = INV_RECORD;
                    error.line = lineNumb;
                    return error;
                }
                contconcCheck(line, inFile, &lineNumb, &error);
            }
        }
        //check if end of file and return error
        else if(feof(inFile)){
            clearList(&tempStore);
            free(line);
            fclose(inFile);
            deleteGEDCOM(temp);
            error.type = INV_GEDCOM;
            return error;
        }
        else{
            customFgets(line, 256, inFile, &error);
            lineNumb++;
            if(error.type != OK){
                deleteGEDCOM(temp);
                clearList(&tempStore);
                free(line);
                fclose(inFile);
                error.type = INV_RECORD;
                error.line = lineNumb;
                return error;
            }
            contconcCheck(line, inFile, &lineNumb, &error);
        }

    }

    //create families for GEDCOM
    createFamilies(temp,fileName,tempStore,&error);

    if(error.type != OK){
        deleteGEDCOM(temp);
        clearList(&tempStore);
        free(line);
        fclose(inFile);
        return error;
    }

    //clear local variables and return object
    clearList(&tempStore);
    free(line);
    fclose(inFile);
    *(obj) = temp;

    error.type = OK;
    return error;

}


/** Function to create a string representation of a GEDCOMobject.
 *@pre GEDCOMobject object exists, is not null, and is valid
 *@post GEDCOMobject has not been modified in any way, and a string representing the GEDCOM contents has been created
 *@return a string contaning a humanly readable representation of a GEDCOMobject
 *@param obj - a pointer to a GEDCOMobject struct
 **/
char* printGEDCOM(const GEDCOMobject* obj){

    if(obj == NULL){
        return NULL;
    }

    //create readable version of GEDCOM
    char* toReturn = malloc(sizeof(char) * 100000);
    char* temp;

    strcpy(toReturn, "GEDCOM object:\n");
    strcat(toReturn, "Header:\n");
    strcat(toReturn, "Header Source:\n");
    strcat(toReturn, obj->header->source);
    char tempged[25];
    sprintf(tempged,"\nGEDCOM version:\n%.2f\n", obj->header->gedcVersion);
    strcat(toReturn, tempged);
    strcat(toReturn, "Header Fields:\n");
    temp = toString(obj->header->otherFields);
    strcat(toReturn, temp);
    free(temp);
    strcat(toReturn, "\nSubmitter:\n");
    strcat(toReturn, "\nSubmitter Name:\n");
    strcat(toReturn, obj->submitter->submitterName);
    strcat(toReturn, "\nSubmitter Address:\n");
    if(strlen(obj->submitter->address) != 0){
       strcat(toReturn, obj->submitter->address);
    }
    strcat(toReturn, "\nSubmitter Fields:\n");
    temp = toString(obj->submitter->otherFields);
    strcat(toReturn, temp);
    free(temp);
    strcat(toReturn,"\nIndividuals:\n");
    temp = toString(obj->individuals);
    strcat(toReturn, temp);
    free(temp);
    strcat(toReturn,"\nFamilies:\n");
    ListIterator iter = createIterator(obj->families);
    while(iter.current != NULL){
        char* temp = printFamily(iter.current->data);
        strcat(toReturn, temp);
        free(temp);
        nextElement(&iter);
    }

    return toReturn;
}


/** Function to delete all GEDCOM object content and free all the memory.
 *@pre GEDCOM object exists, is not null, and has not been freed
 *@post GEDCOM object had been freed
 *@return none
 *@param obj - a pointer to a GEDCOMobject struct
 **/
void deleteGEDCOM(GEDCOMobject* obj){

    //delete all members of GEDCOM object and than the object itself
    if(obj == NULL){
        return;
    }
    if(obj->submitter != NULL){
        clearList(&obj->submitter->otherFields);
        free(obj->submitter);
    }
    if(obj->header != NULL){
        clearList(&obj->header->otherFields);
        free(obj->header);
    }
    clearList(&obj->individuals);
    clearList(&obj->families);
    free(obj);
    obj = NULL;
}


/** Function to "convert" the GEDCOMerror into a humanly redabale string.
 *@return a string contaning a humanly readable representation of the error code
 *@param err - an error struct
 **/
char* printError(GEDCOMerror err){

  char str[15];

  //find error type
  switch(err.type) {
    case OK:
      strcpy(str,"OK");
      break;
    case INV_FILE:
      strcpy(str,"Invalid File");
      break;
    case INV_GEDCOM:
      strcpy(str,"Invalid GEDCOM");
      break;
    case INV_HEADER:
      strcpy(str,"Invalid Header");
      break;
    case INV_RECORD:
      strcpy(str,"Invalid record");
      break;
    case OTHER_ERROR:
      strcpy(str,"Unknown Error");
      break;
    default:
      strcpy(str,"Unknown Error");
      break;
  }
  
  //create readable error
  char* toReturn = malloc(sizeof(char) * 52);
  strcpy(toReturn, str);
  if(err.line != -1){
	  char line[15];
	  sprintf(line,": line %d", err.line);
	  strcat(toReturn, line);
  }
  strcat(toReturn,"\0");

  return toReturn;
}

/** Function that searches for an individual in the list using a comparator function.
 * If an individual is found, a pointer to the Individual record
 * Returns NULL if the individual is not found.
 *@pre GEDCOM object exists,is not NULL, and is valid.  Comparator function has been provided.
 *@post GEDCOM object remains unchanged.
 *@return The Individual record associated with the person that matches the search criteria.  If the Individual record is not found, return NULL.
 *If multiple records match the search criteria, return the first one.
 *@param familyRecord - a pointer to a GEDCOMobject struct
 *@param compare - a pointer to comparator fuction for customizing the search
 *@param person - a pointer to search data, which contains seach criteria
 *Note: while the arguments of compare() and person are all void, it is assumed that records they point to are
 *      all of the same type - just like arguments to the compare() function in the List struct
 **/
Individual* findPerson(const GEDCOMobject* familyRecord, bool (*compare)(const void* first, const void* second), const void* person){

    if(familyRecord == NULL){
        return NULL;
    }

    if(person == NULL){
        return NULL;
    }

    if(compare == NULL){
        return NULL;
    }

    //find person and return him
    void* toFind = findElement(familyRecord->individuals, compare, person);

    if(toFind == NULL){
        return NULL;
    }

    return (Individual*)toFind;
}


/** Function to return a list of all descendants of an individual in a GEDCOM
 *@pre GEDCOM object exists, is not null, and is valid
 *@post GEDCOM object has not been modified in any way, and a list of descendants has been created
 *@return a list of descendants.  The list may be empty.  All list members must be of type Individual, and can appear in any order.
 *All list members must be COPIES of the Individual records in the GEDCOM file.  If the returned list is freed, the original GEDCOM
 *must remain unaffected.
 *@param failyRecord - a pointer to a GEDCOMobject struct
 *@param person - the Individual record whose descendants we want
 **/
List getDescendants(const GEDCOMobject* familyRecord, const Individual* person){
    List descendants = initializeList(&printIndividual, &deleteIndividual, &compareIndividuals);

   if(familyRecord == NULL){
        return descendants;
    }

    if(person == NULL){
        return descendants;
    }

    //create iterator for persons family
    ListIterator iter = createIterator(person->families);
    while(iter.current != NULL){
        //check if person a adult of family
        if((compareIndividuals(((Family*)iter.current->data)->husband,person) == 0 && cmpEvent(((Family*)iter.current->data)->husband,person)) || 
            (compareIndividuals(((Family*)iter.current->data)->wife,person) == 0 && cmpEvent(((Family*)iter.current->data)->wife,person))){
            //copy all children of the family
            ListIterator iter1 = createIterator(((Family*)iter.current->data)->children);
            while(iter1.current != NULL){
                insertBack(&descendants, copyIndi((Individual*)iter1.current->data));
                //recursive call to get descendants
                List temp = getDescendants(familyRecord, (Individual*)iter1.current->data);
                
                //copy all descendants
                ListIterator iter2 = createIterator(temp);
                while(iter2.current != NULL){
                    insertBack(&descendants, iter2.current->data); 
                    nextElement(&iter2);
                }
                
                temp.deleteData = &dummyDelete;
                clearList(&temp);

                nextElement(&iter1);
            }
        }
        nextElement(&iter);
    }

    return descendants;
}



// ****************************** A2 functions ******************************

/** Function to writing a GEDCOMobject into a file in GEDCOM format.
 *@pre GEDCOMobject object exists, is not null, and is valid
 *@post GEDCOMobject has not been modified in any way, and a file representing the
 GEDCOMobject contents in GEDCOM format has been created
 *@return the error code indicating success or the error encountered when parsing the calendar
 *@param obj - a pointer to a GEDCOMobject struct
 **/
GEDCOMerror writeGEDCOM(char* fileName, const GEDCOMobject* obj){
    GEDCOMerror error;
    if(fileName == NULL || obj == NULL){
        error.type = WRITE_ERROR;
        return error;
    }

    int indCount = 0;
    int famCount = 0;

    List tempStore = initializeList(&printIndividual, &destroyNodeData, &compareIndividuals);
    List tempFam = initializeList(&printIndividual, &destroyNodeData, &compareIndividuals);

    FILE* outFile = fopen(fileName, "w");

    if(outFile == NULL){
        error.type = WRITE_ERROR;
        return error;
    }

    fprintf(outFile, "0 HEAD\n");
    if(strlen(obj->header->source) == 0){
        error.type = WRITE_ERROR;
        return error;
    }
    fprintf(outFile, "1 SOUR %s\n", obj->header->source);
    fprintf(outFile, "1 GEDC\n");
    fprintf(outFile, "2 VERS %.2lf\n", obj->header->gedcVersion);
    if(obj->header->gedcVersion == 0){
        error.type = WRITE_ERROR;
        return error;
    }
    fprintf(outFile, "2 FORM LINEAGE-LINKED\n");
    if(obj->header->encoding == ANSEL){
        fprintf(outFile, "1 CHAR ANSEL\n");
    }
    else if(obj->header->encoding == UTF8){
        fprintf(outFile, "1 CHAR UTF-8\n");
    }
    else if(obj->header->encoding == UNICODE){
        fprintf(outFile, "1 CHAR UNICODE\n");
    }
    else if(obj->header->encoding == ASCII){
        fprintf(outFile, "1 CHAR ASCII\n");
    }
    fprintf(outFile, "1 SUBM @SUBM1@\n");
    fprintf(outFile, "0 @SUBM1@ SUBM\n");
    fprintf(outFile, "1 NAME %s\n", obj->submitter->submitterName);
    if(strcmp(obj->submitter->address,"") != 0){
        fprintf(outFile, "1 ADDR %s\n", obj->submitter->address);
    }

    ListIterator iter = createIterator(obj->individuals);
    while(iter.current != NULL){
        Individual* indi = (Individual*)iter.current->data;
        fprintf(outFile, "0 @I%04d@ INDI\n", indCount);

        storeIndi* tempindi = malloc(sizeof(storeIndi));
        tempindi->num = indCount;
        tempindi->temp = indi;

        insertBack(&tempStore, tempindi);

        fprintf(outFile, "1 NAME %s /%s/\n", indi->givenName, indi->surname);
        if(findElement(indi->otherFields, &findTag ,"GIVN") != NULL){
            if(strlen(indi->givenName)==0){
                fprintf(outFile, "2 GIVN Unknown\n");
            }
            else{
                fprintf(outFile, "2 GIVN %s\n", indi->givenName);
            }
        }
        if(findElement(indi->otherFields, &findTag ,"SURN") != NULL){
            if(strlen(indi->surname)==0){
                fprintf(outFile, "2 SURN Unknown\n");
            }
            else{
                fprintf(outFile, "2 SURN %s\n", indi->surname);
            }
        }
        ListIterator fieldIter = createIterator(indi->otherFields);
        while(fieldIter.current != NULL){
            Field* indiField = (Field*)fieldIter.current->data;
            if(strcmp(indiField->tag,"GIVN") != 0 && strcmp(indiField->tag,"SURN") != 0){
                fprintf(outFile, "1 %s %s\n", indiField->tag, indiField->value);
            }
            nextElement(&fieldIter);
        }
        ListIterator eventIter = createIterator(indi->events);
        while(eventIter.current != NULL){
            Event* indiEvent = (Event*)eventIter.current->data;
            fprintf(outFile, "1 %s\n", indiEvent->type);
            if(strcmp(indiEvent->date,"") != 0){
                fprintf(outFile, "2 DATE %s\n", indiEvent->date);
            }
            if(strcmp(indiEvent->place,"") != 0){
                fprintf(outFile, "2 PLAC %s\n", indiEvent->place);
            }
            nextElement(&eventIter);
        }

        ListIterator familyIter = createIterator(indi->families);
        while(familyIter.current != NULL){
            Family* indiFamily = (Family*)familyIter.current->data;
            if(findElement(tempFam,&findFamily,indiFamily) == NULL){
                storeFam* tempfam = malloc(sizeof(storeFam));
                tempfam->num = famCount;
                famCount++;
                tempfam->temp = indiFamily;
                insertBack(&tempFam, tempfam);
            }
            if(indiFamily->husband != NULL){
                if(compareIndividuals(indiFamily->husband,indi) == 0){
                    fprintf(outFile, "1 FAMS @F%03d@\n", ((storeFam*)findElement(tempFam,&findFamily,indiFamily))->num);
                }
            }
            else if(indiFamily->wife != NULL){
                if(compareIndividuals(indiFamily->wife,indi) == 0){
                    fprintf(outFile, "1 FAMS @F%03d@\n", ((storeFam*)findElement(tempFam,&findFamily,indiFamily))->num);
                }

            }
            else{
                fprintf(outFile, "1 FAMC @F%03d@\n", ((storeFam*)findElement(tempFam,&findFamily,indiFamily))->num);
            }
            nextElement(&familyIter);
        }        
        nextElement(&iter);
        indCount++;
    }

    ListIterator familyIter = createIterator(obj->families);
    while(familyIter.current != NULL){
        Family* family = (Family*)familyIter.current->data;
        Individual* husband = family->husband;
        Individual* wife = family->wife;
        if(findElement(tempFam,&findFamily,family) == NULL){
            fprintf(outFile, "0 @F000@ FAM\n");
        }
        else{
            fprintf(outFile, "0 @F%03d@ FAM\n", ((storeFam*)findElement(tempFam,&findFamily,family))->num);
        }
        if(husband != NULL){
            fprintf(outFile, "1 HUSB @I%04d@\n", ((storeIndi*)findElement(tempStore,&findIndividual,husband))->num);
        }
        if(wife != NULL){
            fprintf(outFile, "1 WIFE @I%04d@\n", ((storeIndi*)findElement(tempStore,&findIndividual,wife))->num);
        }

        /*ListIterator eventIter = createIterator(family->events);
        while(eventIter.current != NULL){
            Event* indiEvent = (Event*)eventIter.current->data;
            fprintf(outFile, "1 %s\n", indiEvent->type);
            if(strcmp(indiEvent->date,"") == 0){
                fprintf(outFile, "2 DATE %s\n", indiEvent->date);
            }
            if(strcmp(indiEvent->date,"") == 0){
                fprintf(outFile, "2 PLAC %s\n", indiEvent->place);
            }
            nextElement(&eventIter);
        }*/

        ListIterator childIter = createIterator(family->children);
        while(childIter.current != NULL){
            Individual* child = (Individual*)childIter.current->data;
            fprintf(outFile, "1 CHIL @I%04d@\n", ((storeIndi*)findElement(tempStore,&findIndividual,child))->num);
            nextElement(&childIter);
        }

        nextElement(&familyIter);
    }

    fprintf(outFile, "0 TRLR\n");

    error.type = OK;
    fclose(outFile);
    clearList(&tempFam);
    clearList(&tempStore);

    return error;
}

/** Function for validating an existing GEDCOM object
 *@pre GEDCOM object exists and is not null
 *@post GEDCOM object has not been modified in any way
 *@return the error code indicating success or the error encountered when validating the GEDCOM
 *@param obj - a pointer to a GEDCOMobject struct
 **/
ErrorCode validateGEDCOM(const GEDCOMobject* obj){
    if(obj == NULL){
        return INV_GEDCOM;
    }

    if(obj->header == NULL || obj->submitter == NULL){
        return INV_GEDCOM;
    }

    if(strlen(obj->header->source) == 0 || obj->header->gedcVersion == 0 || obj->header->submitter == NULL){
        return INV_HEADER;
    }

    if(strlen(obj->submitter->submitterName) == 0){
        return INV_RECORD;
    }

   /* ListIterator iter = createIterator(obj->individuals);
    for (int i = 0; i < obj->individuals.length; ++i){
        Individual* temp = (Individual*)iter.current->data;
        if(temp == NULL){
            return INV_RECORD;
        }
        if(strlen(temp->givenName) > 200 ||  strlen(temp->surname) > 200){
            return INV_RECORD;
        }

        ListIterator eventiter = createIterator(temp->events);
        for (int i = 0; i < temp->events.length; ++i){
            Event* event = (Event*)eventiter.current->data;
            if(event == NULL){
                return INV_RECORD;
            }
            if(event->place == NULL || strlen(event->place)>200 || event->date == NULL || strlen(event->date)>200){
                return INV_RECORD;
            }
            ListIterator fielditer = createIterator(event->otherFields);
            for (int i = 0; i < event->otherFields.length; ++i){
                Field* field = (Field*)fielditer.current->data;
                if(field == NULL){
                    return INV_RECORD;
                }
                if(field->tag == NULL || strlen(field->tag)>32 || field->value == NULL || strlen(field->value)>200){
                    return INV_RECORD;
                }
                nextElement(&fielditer);
            }

            nextElement(&eventiter);
        }

        ListIterator fielditer = createIterator(temp->otherFields);
        for (int i = 0; i < temp->otherFields.length; ++i){
            Field* field = (Field*)fielditer.current->data;
            if(field == NULL){
                return INV_RECORD;
            }
            if(field->tag == NULL || strlen(field->tag)>32 || field->value == NULL || strlen(field->value)>200){
                return INV_RECORD;
            }
            nextElement(&fielditer);
        }

        nextElement(&iter);
    }

    ListIterator iter2 = createIterator(obj->families);
    for (int i = 0; i < obj->families.length; ++i){
        Family* temp = (Family*)iter2.current->data;
        if(temp == NULL){
            return INV_RECORD;
        }

        ListIterator childiter = createIterator(temp->children);
        for (int i = 0; i < temp->children.length; ++i){
            Individual* temp = (Individual*)childiter.current->data;
            if(temp == NULL){
                return INV_RECORD;
            }
            nextElement(&childiter);
        }

        ListIterator eventiter = createIterator(temp->events);
        for (int i = 0; i < temp->events.length; ++i){
            Event* event = (Event*)eventiter.current->data;
            if(event == NULL){
                return INV_RECORD;
            }
            if(event->place == NULL || strlen(event->place)>200 || event->date == NULL || strlen(event->date)>200){
                return INV_RECORD;
            }
            ListIterator fielditer = createIterator(event->otherFields);
            for (int i = 0; i < event->otherFields.length; ++i){
                Field* field = (Field*)fielditer.current->data;
                if(field == NULL){
                    return INV_RECORD;
                }
                if(field->tag == NULL || strlen(field->tag)>32 || field->value == NULL || strlen(field->value)>200){
                    return INV_RECORD;
                }
                nextElement(&fielditer);
            }

            nextElement(&eventiter);
        }


        ListIterator fielditer = createIterator(temp->otherFields);
        for (int i = 0; i < temp->otherFields.length; ++i){
            Field* field = (Field*)fielditer.current->data;
            if(field == NULL){
                return INV_RECORD;
            }
            if(field->tag == NULL || strlen(field->tag)>32 || field->value == NULL || strlen(field->value)>200){
                return INV_RECORD;
            }
            nextElement(&fielditer);
        }
        nextElement(&iter2);
    }*/


    return OK;
}

/** Function to return a list of up to N generations of descendants of an individual in a GEDCOM
 *@pre GEDCOM object exists, is not null, and is valid
 *@post GEDCOM object has not been modified in any way, and a list of descendants has been created
 *@return a list of descendants.  The list may be empty.  All list members must be of type List.    *@param familyRecord - a pointer to a GEDCOMobject struct
 *@param person - the Individual record whose descendants we want
 *@param maxGen - maximum number of generations to examine (must be >= 1)
 **/


List getDescendantListN(const GEDCOMobject* familyRecord, const Individual* person, unsigned int maxGen){

    List descendants = initializeList(&printGeneration, &deleteGeneration, &compareGenerations);

    if(person == NULL){
        return descendants;
    } 
            
    Family *family;
    ListIterator iter = createIterator(person->families);
    while(iter.current != NULL){
        family = (Family*)iter.current->data;
        if(compareIndividuals(family->wife,person) == 0  || compareIndividuals(family->husband,person) == 0){
            recursiveDescendant(&descendants, family, maxGen, 1);
        }

        nextElement(&iter);
    }

    return descendants;

}

/** Function to return a list of up to N generations of ancestors of an individual in a GEDCOM
 *@pre GEDCOM object exists, is not null, and is valid
 *@post GEDCOM object has not been modified in any way, and a list of ancestors has been created
 *@return a list of ancestors.  The list may be empty.
 *@param familyRecord - a pointer to a GEDCOMobject struct
 *@param person - the Individual record whose descendants we want
 *@param maxGen - maximum number of generations to examine (must be >= 1)
 **/


List getAncestorListN(const GEDCOMobject* familyRecord, const Individual* person, int maxGen){

    List descendants = initializeList(&printGeneration, &deleteGeneration, &compareGenerations);

    if(person == NULL){
        return descendants;
    } 

    Family *family;
    ListIterator iter = createIterator(person->families);
    while(iter.current != NULL){
        family = (Family*)iter.current->data;
        if(family->wife != person && family->husband != person){
            recursiveAnscestor(&descendants, family, maxGen, 1);
        }
        nextElement(&iter);
    }


     return descendants;

}

/** Function for converting an Individual struct into a JSON string
 *@pre Individual exists, is not null, and is valid
 *@post Individual has not been modified in any way, and a JSON string has been created
 *@return newly allocated JSON string.  May be NULL.
 *@param ind - a pointer to an Individual struct
 **/
char* indToJSON(const Individual* ind){
    if(ind == NULL){
        char* toReturn = malloc(sizeof(char));
        strcpy(toReturn,"\0");
        return toReturn;
    }

    char* toReturn = malloc(sizeof(char)* 302);

    strcpy(toReturn, "{\"givenName\":\"\0");
    if(ind->givenName == NULL){
    }
    else{
        strcat(toReturn, ind->givenName);
    }
    strcat(toReturn, "\",\"surname\":\"");
    if(ind->surname == NULL){
    }
    else{
        strcat(toReturn, ind->surname);
    }
    strcat(toReturn, "\"}");

    return toReturn;
}

/** Function for creating an Individual struct from an JSON string
 *@pre String is not null, and is valid
 *@post String has not been modified in any way, and an Individual struct has been created
 *@return a newly allocated Individual struct.  May be NULL.
 *@param str - a pointer to a JSON string
 **/
Individual* JSONtoInd(const char* str){
    if(str == NULL){
        return NULL;
    }

    if(strlen(str) == 0){
        return NULL;
    }

    if(str[0] != '{'){
        return NULL;
    }

    Individual* indi = malloc(sizeof(Individual));

    char* temp  = malloc(sizeof(char) * (strlen(str)+1));
    strcpy(temp,str);

    char* token;

    strtok(temp,"\"");
    strtok(NULL,"\"");
    strtok(NULL,"\"");
    token = strtok(NULL,"\"");


    if(strcmp(token,",") == 0){
        indi->givenName = malloc(sizeof(char));
        strcpy(indi->givenName, "");
        return indi;
    }
    else{
        indi->givenName = malloc(sizeof(char)* (strlen(token) + 1));
        sprintf(indi->givenName, "%s", token);
    }

    strtok(NULL,"\"");
    strtok(NULL,"\"");
    strtok(NULL,"\"");
    token = strtok(NULL,"\"");

    if(strcmp(token,"}") == 0){
        indi->surname = malloc(sizeof(char));
        strcpy(indi->surname, "");
        return indi;
    }

    indi->surname = malloc(sizeof(char)* (strlen(token) + 1));
    sprintf(indi->surname, "%s", token);

    return indi;
}

void JSONaddindi(char* fileName, char* firstname, char* lastname){
    GEDCOMobject* gedcomObject = NULL;
    createGEDCOM(fileName, &gedcomObject);

    Individual* toReturn = malloc(sizeof(Individual));

    toReturn->events = initializeList(&printEvent, &deleteEvent, &compareEvents);
    toReturn->otherFields = initializeList(&printField, &deleteField, &compareFields);
    toReturn->families = initializeList(&printFamily, &deleteFamily, &compareFamilies);

    toReturn->givenName = malloc(sizeof(char)*(strlen(firstname)+1));
    strcpy(toReturn->givenName,firstname);
    toReturn->surname = malloc(sizeof(char)*(strlen(lastname)+1));
    strcpy(toReturn->surname,lastname);

    addIndividual(gedcomObject, toReturn);

    writeGEDCOM(fileName, gedcomObject);
}

char* filterfiles(char* fileName){
    GEDCOMobject* gedcomObject = NULL;

    char* toReturn = calloc(50, sizeof(char));
    createGEDCOM(fileName, &gedcomObject);

    if(validateGEDCOM(gedcomObject) == OK){
        strcpy(toReturn, "OK");
    }
    else{
        strcpy(toReturn, "NOTOK");
    }

    return toReturn;

}


char* GEDCOMtoJSON(char* fileName){
    GEDCOMobject* gedcomObject = NULL;

    char* toReturn = calloc(5000, sizeof(char));
    createGEDCOM(fileName, &gedcomObject);
    strcpy(toReturn, "{\"source\":\"\0");
    strcat(toReturn,gedcomObject->header->source);
    strcat(toReturn, "\",\"version\":\"");
    char buffer[10];
    sprintf(buffer,"%.2f\",", gedcomObject->header->gedcVersion);
    strcat(toReturn, buffer);
    strcat(toReturn, "\"encoding\":\"");
    if(gedcomObject->header->encoding == ANSEL){
        strcat(toReturn, "ANSEL\",");
    }
    else if(gedcomObject->header->encoding == UTF8){
        strcat(toReturn, "UTF-8\",");
    }
    else if(gedcomObject->header->encoding == UNICODE){
        strcat(toReturn, "UNICODE\",");
    }
    else if(gedcomObject->header->encoding == ASCII){
        strcat(toReturn, "ASCII\",");
    } 
    strcat(toReturn, "\"name\":\"");
    strcat(toReturn,gedcomObject->submitter->submitterName);
    strcat(toReturn, "\",\"adress\":\"");
    if(strlen(gedcomObject->submitter->address) != 0){
        strcat(toReturn,gedcomObject->submitter->address);
    }
    strcat(toReturn, "\",");
    char tempnumbers[200];
    sprintf(tempnumbers, "\"indi\":\"%d\",\"fam\":\"%d\"", gedcomObject->individuals.length, gedcomObject->families.length);
    strcat(toReturn, tempnumbers);
    strcat(toReturn, "}");
    /*deleteGEDCOM(gedcomObject);*/
    return toReturn;
    
}


char* createIndJSON(char* fileName){
    GEDCOMobject* gedcomObject = NULL;
    createGEDCOM(fileName, &gedcomObject);

    char *indList = iListToJSON(gedcomObject->individuals);
    return indList;
}


void saveGEDCOM(char* fileName, char* submname, char* submadress){
    GEDCOMobject* toReturn = malloc(sizeof(GEDCOMobject));
    toReturn->individuals = initializeList(&printIndividual, &deleteIndividual, &compareIndividuals);
    toReturn->families = initializeList(&printFamily, &deleteFamily, &compareFamilies);

    Header* header = malloc(sizeof(Header));
    header->otherFields = initializeList(&printField, &deleteField, &compareFields);

    header->encoding = ANSEL;
    header->gedcVersion = 5.5;
    strcpy(header->source, "genealogy");


    Submitter* submitter = malloc(sizeof(Submitter));
    submitter->otherFields = initializeList(&printField, &deleteField, &compareFields);

    strcpy(submitter->submitterName,submname);

    /*if(submadress != NULL && strlen(submadress)==0){
        realloc()
        strcpy(submitter->address,submadress);
    }*/

    header->submitter = submitter;
    toReturn->submitter = submitter;

    toReturn->header = header;

    writeGEDCOM(fileName, toReturn);

    deleteGEDCOM(toReturn);
}

char* JSONdescendants(char* filename, char* firstname, char* lastname, int num){
    GEDCOMobject* gedcomObject = NULL;

    createGEDCOM(filename, &gedcomObject);

    Individual* indi = malloc(sizeof(Individual));

    indi->givenName = malloc(sizeof(char)*(strlen(firstname)+1));
    sprintf(indi->givenName,"%s",firstname);
    indi->surname = malloc(sizeof(char)*(strlen(lastname)+1));
    sprintf(indi->surname,"%s",lastname);

    List descendants = getDescendantListN(gedcomObject, indi, num);

    deleteGEDCOM(gedcomObject);

    char* temp = gListToJSON(descendants);

    clearList(&descendants);

    return temp;
}

char* JSONancestors(char* filename, char* firstname, char* lastname, int num){
    GEDCOMobject* gedcomObject = NULL;

    createGEDCOM(filename, &gedcomObject);

    Individual* indi = malloc(sizeof(Individual));

    indi->givenName = malloc(sizeof(char)*(strlen(firstname)+1));
    sprintf(indi->givenName,"%s",firstname);
    indi->surname = malloc(sizeof(char)*(strlen(lastname)+1));
    sprintf(indi->surname,"%s",lastname);

    List descendants = getAncestorListN(gedcomObject, indi, num);

    deleteGEDCOM(gedcomObject);

    char* temp = gListToJSON(descendants);

    clearList(&descendants);

    return temp;
}

/** Function for creating a GEDCOMobject struct from an JSON string
 *@pre String is not null, and is valid
 *@post String has not been modified in any way, and a GEDCOMobject struct has been created
 *@return a newly allocated GEDCOMobject struct.  May be NULL.
 *@param str - a pointer to a JSON string
 **/
GEDCOMobject* JSONtoGEDCOM(const char* str){

    if(str == NULL){
        return NULL;
    }

    if(strlen(str) == 0){
        return NULL;
    }

    if(str[0] != '{'){
        return NULL;
    }

    GEDCOMobject* toReturn = malloc(sizeof(GEDCOMobject));
    toReturn->individuals = initializeList(&printIndividual, &deleteIndividual, &compareIndividuals);
    toReturn->families = initializeList(&printFamily, &deleteFamily, &compareFamilies);

    char* temp  = malloc(sizeof(char) * (strlen(str)+1));
    strcpy(temp,str);

    char* token;

    strtok(temp,"\"");
    strtok(NULL,"\"");
    strtok(NULL,"\"");
    token = strtok(NULL,"\"");

    Header* header = malloc(sizeof(Header));
    header->otherFields = initializeList(&printField, &deleteField, &compareFields);

    sprintf(header->source, "%s", token);

    strtok(NULL,"\"");
    strtok(NULL,"\"");
    strtok(NULL,"\"");
    token = strtok(NULL,"\"");

    header->gedcVersion = atof(token);

    strtok(NULL,"\"");
    strtok(NULL,"\"");
    strtok(NULL,"\"");
    token = strtok(NULL,"\"");

    if(strncmp(token,"ANSEL", 5) == 0){
        header->encoding = ANSEL;
    }
    else if(strncmp(token,"UTF-8", 5) == 0){
        header->encoding = UTF8;
    }
    else if(strncmp(token,"UNICODE", 7) == 0){
        header->encoding = UNICODE;
    }
    else if(strncmp(token,"ASCII", 5) == 0){
        header->encoding = ASCII;
    }    

    Submitter* submitter = malloc(sizeof(Submitter) + sizeof(char) * 255);
    submitter->otherFields = initializeList(&printField, &deleteField, &compareFields);

    strtok(NULL,"\"");
    strtok(NULL,"\"");
    strtok(NULL,"\"");
    token = strtok(NULL,"\"");

    if(token == NULL || strlen(token) == 0){
        return NULL;
    }

    sprintf(submitter->submitterName, "%s", token);

    strtok(NULL,"\"");
    strtok(NULL,"\"");
    strtok(NULL,"\"");
    token = strtok(NULL,"\"");

    if(token == NULL){
        return NULL;
    }

    if(strcmp(token,"}") != 0){
        sprintf(submitter->address, "%s", token);
    }

    header->submitter = submitter;
    toReturn->submitter = submitter;

    toReturn->header = header;

    return toReturn;
}

/** Function for adding an Individual to a GEDCCOMobject
 *@pre both arguments are not NULL and valid
 *@post Individual has not been modified in any way, and its address had been added to GEDCOMobject's individuals list
 *@return void
 *@param obj - a pointer to a GEDCOMobject struct
 *@param toBeAdded - a pointer to an Individual struct
**/
void addIndividual(GEDCOMobject* obj, const Individual* toBeAdded){
    if(obj == NULL || toBeAdded == NULL){
        return;
    }

    insertBack(&obj->individuals, (void*)toBeAdded);

}

/** Function for converting a list of Individual structs into a JSON string
 *@pre List exists, is not null, and has been initialized
 *@post List has not been modified in any way, and a JSON string has been created
 *@return newly allocated JSON string.  May be NULL.
 *@param iList - a pointer to a list of Individual structs
 **/
char* iListToJSON(List iList){
    char* toReturn = malloc(sizeof(char)* 10000);

    if(iList.length == 0){
        strcpy(toReturn,"[]\0");
    }

    strcpy(toReturn,"[\0");

    ListIterator iter = createIterator(iList);
    while(iter.current != NULL){
        char* temp = indToJSON(iter.current->data);
        strcat(toReturn, temp);
        free(temp);

        nextElement(&iter);

        if(iter.current != NULL){
            strcat(toReturn, ",");
        }
    }


    strcat(toReturn ,"]");

    return toReturn;
}

/** Function for converting a list of lists of Individual structs into a JSON string
 *@pre List exists, is not null, and has been initialized
 *@post List has not been modified in any way, and a JSON string has been created
 *@return newly allocated JSON string.  May be NULL.
 *@param gList - a pointer to a list of lists of Individual structs
 **/
char* gListToJSON(List gList){

    char* toReturn = malloc(sizeof(char)* 10000);

    if(gList.length == 0){
        strcpy(toReturn,"[]\0");
        return toReturn;
    }

    strcpy(toReturn,"[\0");


    ListIterator iter = createIterator(gList);
    while(iter.current != NULL){
        char* temp = iListToJSON(*(List*)iter.current->data);
        strcat(toReturn, temp);
        free(temp);

        nextElement(&iter);
        
        if(iter.current != NULL){
            strcat(toReturn, ",");
        }
    }

    strcat(toReturn ,"]");

    return toReturn;

}


//****************************************** List helper functions added for A2 *******************************************
void deleteGeneration(void* toBeDeleted){
    clearList((List*)toBeDeleted);
}

int compareGenerations(const void* first,const void* second){
    return 1;
}

char* printGeneration(void* toBePrinted){
    int generation = 1;
    char* toReturn = malloc(sizeof(char)*5000);
    
    sprintf(toReturn, "Generation %d:\n", generation);
    char* temp = toString(((Event*)toBePrinted)->otherFields);
    strcat(toReturn, temp);
    free(temp);

    return toReturn;
}




//************************************************************************************************************

//****************************************** List helper functions *******************************************
void deleteEvent(void* toBeDeleted){
    free(((Event*)toBeDeleted)->date);
    free(((Event*)toBeDeleted)->place);
    clearList(&((Event*)toBeDeleted)->otherFields);
    free((Event*)toBeDeleted);
}

int compareEvents(const void* first,const void* second){
    return strcmp(((Event*)first)->type, ((Event*)second)->type);
}

char* printEvent(void* toBePrinted){
	char* toReturn = malloc(sizeof(char)*502);
	
	strcpy(toReturn, "Event:\nType: ");
	strcat(toReturn, ((Event*)toBePrinted)->type);
	strcat(toReturn, "\nDate: ");
	strcat(toReturn, ((Event*)toBePrinted)->date);
	strcat(toReturn, "\nPlace: ");
	strcat(toReturn, ((Event*)toBePrinted)->place);
	strcat(toReturn, "\nEvent Fields:\n");
    char* temp = toString(((Event*)toBePrinted)->otherFields);
    strcat(toReturn, temp);
    free(temp);

    return toReturn;
}

void deleteIndividual(void* toBeDeleted){
    free(((Individual*)toBeDeleted)->givenName);
    free(((Individual*)toBeDeleted)->surname);
    clearList(&((Individual*)toBeDeleted)->events);
    clearList(&((Individual*)toBeDeleted)->otherFields);
    clearList(&((Individual*)toBeDeleted)->families);
    free((Individual*)toBeDeleted);
}

int compareIndividuals(const void* first,const void* second){
    char firstCat[102] = "\0";
    char secondCat[102] = "\0";

    strcat(firstCat,((Individual*)first)->surname);
    strcat(firstCat,",");
    strcat(firstCat,((Individual*)first)->givenName);

    strcat(secondCat,((Individual*)second)->surname);
    strcat(secondCat,",");
    strcat(secondCat,((Individual*)second)->givenName);

    return strcmp(firstCat,secondCat);
}

char* printIndividual(void* toBePrinted){
	char* toReturn = malloc(sizeof(char)*2002);
	
	strcpy(toReturn, "Individual:\nName: ");
	strcat(toReturn, ((Individual*)toBePrinted)->givenName);
	strcat(toReturn, (" "));
	strcat(toReturn, ((Individual*)toBePrinted)->surname);
	strcat(toReturn, "\n\nEvents:\n");
    char* temp = toString(((Individual*)toBePrinted)->events);
    strcat(toReturn, temp);
    free(temp);
    strcat(toReturn, "\nIndividual Fields:\n");
    temp = toString(((Individual*)toBePrinted)->otherFields);
    strcat(toReturn, temp);
    free(temp);
    strcat(toReturn, "\n");
	
    return toReturn;
}

void deleteFamily(void* toBeDeleted){
    clearList(&((Family*)toBeDeleted)->otherFields);
    clearList(&((Family*)toBeDeleted)->children);
    clearList(&((Family*)toBeDeleted)->events);
    free((Family*)toBeDeleted);
}

int compareFamilies(const void* first,const void* second){
    int firstNum = 0;
    int secondNum = 0;

    if(((Family*)first)->wife != NULL){
        firstNum++;
    }
    if(((Family*)first)->husband != NULL){
        firstNum++;
    }
    firstNum += ((Family*)first)->children.length;

    if(((Family*)second)->wife != NULL){
        secondNum++;
    }
    if(((Family*)second)->husband != NULL){
        secondNum++;
    }
    secondNum += ((Family*)second)->children.length;

    if(firstNum>secondNum){
        return 1;
    }
    else if(firstNum<secondNum){
        return -1;
    }

    return 0;

}

char* printFamily(void* toBePrinted){
	char* toReturn = malloc(sizeof(char)*2002);
	strcpy(toReturn, "\n\nFamily:\nHusband:\n\0");
    char* temp;
    if(((Family*)toBePrinted)->husband != NULL){
        temp = printName(((Family*)toBePrinted)->husband);
	    strcat(toReturn,temp);
        free(temp);
    }
	strcat(toReturn, "\nWife:\n");
    if(((Family*)toBePrinted)->wife != NULL){
        temp = printName(((Family*)toBePrinted)->wife);
	    strcat(toReturn,temp);
        free(temp);
    }
	strcat(toReturn, "\nChildren:\n");
    ListIterator iter = createIterator(((Family*)toBePrinted)->children);
    while(iter.current != NULL){
        temp = printName(iter.current->data);
        strcat(toReturn, temp); 
        free(temp);
        strcat(toReturn, "\n");
        nextElement(&iter);
    }
	strcat(toReturn, "Family Fields:");
    temp = toString(((Family*)toBePrinted)->otherFields);
    strcat(toReturn, temp);
    free(temp);
    strcat(toReturn, "\0");
	
    return toReturn;
}

void deleteField(void* toBeDeleted){
    free(((Field*)toBeDeleted)->tag);
    free(((Field*)toBeDeleted)->value);
    free((Field*)toBeDeleted);
}

int compareFields(const void* first,const void* second){
    char firstCat[102] = "\0";
    char secondCat[102] = "\0";

    strcat(firstCat,((Field*)first)->tag);
    strcat(firstCat," ");
    strcat(firstCat,((Field*)first)->value);

    strcat(secondCat,((Field*)second)->tag);
    strcat(secondCat," ");
    strcat(secondCat,((Field*)second)->value);

    return strcmp(firstCat,secondCat);
}

char* printField(void* toBePrinted){

    char* toReturn = malloc(sizeof(char) * 255);
    sprintf(toReturn, "Field:\nTag: %s\nValue: %s\n", ((Field*)toBePrinted)->tag , ((Field*)toBePrinted)->value);

    return toReturn;
}


Event* copyEvent(Event* toCopy){
    //create a new event and copy all indi fields
    Event* event = malloc(sizeof(Event));
    strcpy(event->type, toCopy->type);
    event->date = malloc(sizeof(char)* (strlen(toCopy->date) + 1));
    strcpy(event->date, toCopy->date);
    event->place = malloc(sizeof(char)* (strlen(toCopy->place) + 1));
    strcpy(event->place, toCopy->place);

    ListIterator iter1 = createIterator(toCopy->otherFields);
    while(iter1.current != NULL){
        insertBack(&event->otherFields, copyField((Field*)iter1.current->data));
        nextElement(&iter1);
    }

    return event;
}

Field* copyField(Field* toCopy){
    //create a new field and copy all indi fields
    Field* field = malloc(sizeof(Field));
    field->tag = malloc(sizeof(char)* (strlen(toCopy->tag) + 1));
    strcpy(field->tag, toCopy->tag);
    field->value = malloc(sizeof(char)* (strlen(toCopy->value) + 1));
    strcpy(field->value, toCopy->value);
    return field;
}

Family* copyFamily(Family* toCopy){
    //create a new family and copy all indi fields
    Family* family = malloc(sizeof(Family));
    family->husband = toCopy->husband;
    family->wife = toCopy->wife;
    family->children = initializeList(&printIndividual, &dummyDelete, &compareIndividuals);
    family->otherFields = initializeList(&printField, &deleteField, &compareFields);
    ListIterator iter = createIterator(toCopy->children);
    while(iter.current != NULL){
        insertBack(&family->children, iter.current->data);
        nextElement(&iter);
    }
    ListIterator iter1 = createIterator(toCopy->otherFields);
    while(iter1.current != NULL){
        insertBack(&family->otherFields, copyField((Field*)iter1.current->data));
        nextElement(&iter1);
    }

    return family;
}

Individual* copyIndi(Individual* toCopy){
    //create a new idividual and copy all indi fields
    Individual* toReturn = malloc(sizeof(Individual));

    toReturn->events = initializeList(&printEvent, &deleteEvent, &compareEvents);
    toReturn->otherFields = initializeList(&printField, &deleteField, &compareFields);
    toReturn->families = initializeList(&printFamily, &deleteFamily, &compareFamilies);

    toReturn->givenName = malloc(sizeof(char)*(strlen(toCopy->givenName)+1));
    sprintf(toReturn->givenName,"%s",toCopy->givenName);
    toReturn->surname = malloc(sizeof(char)*(strlen(toCopy->surname)+1));
    sprintf(toReturn->surname,"%s",toCopy->surname);

    ListIterator iter = createIterator(toCopy->events);
    while(iter.current != NULL){
        insertBack(&toReturn->events, copyEvent((Event*)iter.current->data));
        nextElement(&iter);
    }

    ListIterator iter1 = createIterator(toCopy->otherFields);
    while(iter1.current != NULL){
        insertBack(&toReturn->otherFields, copyField((Field*)iter1.current->data));
        nextElement(&iter1);
    }

    ListIterator iter2 = createIterator(toCopy->families);
    while(iter2.current != NULL){
        insertBack(&toReturn->families, copyFamily((Family*)iter2.current->data));
        nextElement(&iter2);
    }
    return toReturn;

}

bool cmpEvent(Individual* a, const Individual* b){
    ListIterator iter = createIterator(a->events);
    ListIterator iter1 = createIterator(b->events);

    //compare all individuals for equality and return t or f
    if(getLength(a->events) != getLength(b->events)){
        return false;
    }

    while(iter.current != NULL && iter1.current!= NULL){
        if(compareEvents((Event*)iter.current->data, (Event*)iter1.current->data) != 0){
            return true;
        }
        nextElement(&iter);
        nextElement(&iter1);
    }

    return true;
}

void createFamilies (GEDCOMobject* temp, char* fileName, List tempStore, GEDCOMerror* error){

    char* token;
    char line[256];
    int lineNumb = 0;

    FILE* inFile = fopen(fileName, "r");
    customFgets(line, 256, inFile, error);
    lineNumb++;
    contconcCheck(line, inFile, &lineNumb, error);

    while(!feof(inFile)){
        if(strncmp(line,"0 TRLR", 6) == 0){
            break;
        }
        //go thru file and filter families
        token = tokenize(line);
        if(token == NULL){
            customFgets(line, 256, inFile, error);
            lineNumb++;
            if(error->type != OK){
                error->type = INV_RECORD;
                error->line = lineNumb;
                fclose(inFile);
                return;
            }
            contconcCheck(line, inFile, &lineNumb, error);
            continue;
        }
        if(strncmp(token,"FAM", 3) != 0){
            customFgets(line, 256, inFile, error);
            lineNumb++;
            if(error->type != OK){
                error->type = INV_RECORD;
                error->line = lineNumb;
                fclose(inFile);
                return;
            }
            contconcCheck(line, inFile, &lineNumb, error);
            continue;
        }
        if(strncmp(token,"FAM", 3) == 0){
            Family* fam = malloc(sizeof(Family));
            fam->children = initializeList(&printIndividual, &dummyDelete, &compareIndividuals);
            fam->events = initializeList(&printEvent, &deleteEvent, &compareEvents);
            fam->otherFields = initializeList(&printField, &deleteField, &compareFields);
            customFgets(line, 256, inFile, error);
            lineNumb++;
            contconcCheck(line, inFile, &lineNumb, error);
            while(line[0] != '0'){
                //check if wife reference
                if(strncmp("1 WIFE",line,6) == 0){
                    token = tokenize(line);
                    if(token == NULL){
                        error->type = INV_RECORD;
                        error->line = lineNumb;
                        deleteFamily(fam);
                        fclose(inFile);
                        return;
                    }
                    void *tempWife = findElement(tempStore, &compareTag, token);
                    //if wife not found means invalid line
                    if(tempWife == NULL){
                        error->type = INV_RECORD;
                        error->line = lineNumb;
                        deleteFamily(fam);
                        fclose(inFile);
                        return;
                    }
                    Individual* wife = ((tagIndi*)tempWife)->temp;
                    //if wife not found means invalid line
                    if(wife == NULL){
                        error->type = INV_RECORD;
                        error->line = lineNumb;
                        deleteFamily(fam);
                        fclose(inFile);
                        return;
                    }
                    //otherwise create reference to wife
                    fam->wife = (Individual*)wife;
                    insertBack(&wife->families, fam);
                }
                //check if husband reference
                else if(strncmp("1 HUSB",line,6) == 0){
                    token = tokenize(line);
                    if(token == NULL){
                        error->type = INV_RECORD;
                        error->line = lineNumb;
                        deleteFamily(fam);
                        fclose(inFile);
                        return;
                    }
                    void *tempHusb = findElement(tempStore, &compareTag, token);
                    //if husband not found means invalid line
                    if(tempHusb == NULL){
                        error->type = INV_RECORD;
                        error->line = lineNumb;
                        deleteFamily(fam);
                        fclose(inFile);
                        return;
                    }
                    Individual* husband = ((tagIndi*)tempHusb)->temp;
                    //otherwise create reference to husband
                    fam->husband = (Individual*)husband;
                    insertBack(&husband->families, fam);
                }
                //check for a child reference
                else if(strncmp("1 CHIL",line,6) == 0){
                    token = tokenize(line);
                    if(token == NULL){
                        error->type = INV_RECORD;
                        error->line = lineNumb;
                        deleteFamily(fam);
                        fclose(inFile);
                        return;
                    }
                    void *tempChild = findElement(tempStore, &compareTag, token);
                    //if child not found return error
                    if(tempChild == NULL){
                        error->type = INV_RECORD;
                        error->line = lineNumb;
                        fclose(inFile);
                        deleteFamily(fam);
                        return;
                    }
                    Individual* child = ((tagIndi*)tempChild)->temp;
                    //otherwise create references to children
                    insertBack(&fam->children,child);
                    insertBack(&child->families, fam);
                }
                else if(strncmp(line,"1 EVEN", 6) == 0 || strncmp(line,"1 MARS",6) == 0 || strncmp(line,"1 MARL",6) == 0 || strncmp(line,"1 MARC",6) == 0 ||
                        strncmp(line,"1 MARB",6) == 0 || strncmp(line,"1 MARR",6) == 0  || strncmp(line,"1 ENGA",6) == 0  || strncmp(line,"1 DIVF",6) == 0  
                        || strncmp(line,"1 DIV",5) == 0 || strncmp(line,"1 CENS",6) == 0 || strncmp(line,"1 ANUL",6) == 0){

                        Event* event = malloc(sizeof(Event));
                        event->otherFields = initializeList(&printField, &deleteField, &compareFields);
                        strncpy(event->type, token, 4);
                        while(1){
                            customFgets(line, 256, inFile, error);
                            lineNumb++;
                            if(error->type != OK){
                                error->type = INV_RECORD;
                                error->line = lineNumb;
                                return;
                            }
                            contconcCheck(line, inFile, &lineNumb, error);
                            //if event record over break loop
                            if(line[0] == '0' || line[0] == '1'){
                                if(event->date == NULL){
                                    event->date = malloc(sizeof(char));
                                    strcpy(event->date,"");
                                }
                                if(event->place == NULL){
                                    event->place = malloc(sizeof(char));
                                    strcpy(event->place,"");
                                }
                                break;
                            }
                            //retrieve event date
                            if(strncmp(line,"2 DATE",6) == 0){
                                token = tokenize(line);
                                if(token == NULL){
                                    error->line = lineNumb;
                                    error->type = INV_RECORD;
                                    return;
                                }
                                event->date = malloc(sizeof(char)* (strlen(token) + 1));
                                sprintf(event->date, "%s", token);
                            }
                            //retrieve event place
                            else if(strncmp(line,"2 PLAC",6) == 0){
                                token = tokenize(line);
                                if(token == NULL){
                                    error->line = lineNumb;
                                    error->type = INV_RECORD;
                                    return;
                                }
                                event->place = malloc(sizeof(char)* (strlen(token) + 1));
                                sprintf(event->place, "%s", token);
                            }
                            else{
                                //create appropariate event field if valid
                                Field* field = malloc(sizeof(Field));
                                token = strtok(line, " ");
                                token = strtok(NULL, " ");
                                if(token == NULL){
                                    error->line = lineNumb;
                                    error->type = INV_RECORD;
                                    continue;
                                }
                                field->tag = malloc(sizeof(char)* (strlen(token) + 1));
                                sprintf(field->tag, "%s", token);
                                token = strtok(NULL, "");
                                if(token == NULL){
                                    error->line = lineNumb;
                                    error->type = INV_RECORD;
                                    continue;
                                }
                                field->value = malloc(sizeof(char)* (strlen(token) + 1));
                                sprintf(field->value, "%s", token);
                                insertBack(&event->otherFields,field);
                            }
                        }

                        insertBack(&fam->events, event);
                        continue;
                }
                else{
                    //insert field into family lists if a valid field
                    Field* field = malloc(sizeof(Field));
                    token = strtok(line, " ");
                    token = strtok(NULL, " ");
                    if(token == NULL){
                        free(field);
                        customFgets(line, 256, inFile, error);
                        lineNumb++;
                        contconcCheck(line, inFile, &lineNumb, error);
                        continue;
                    }
                    field->tag = malloc(sizeof(char)* (strlen(token) + 1));
                    sprintf(field->tag, "%s", token);
                    token = strtok(NULL, "");
                    if(token == NULL){
                        free(field->tag);
                        free(field);
                        customFgets(line, 256, inFile, error);
                        lineNumb++;
                        contconcCheck(line, inFile, &lineNumb, error);
                        continue;
                    }
                    field->value = malloc(sizeof(char)* (strlen(token) + 1));
                    sprintf(field->value, "%s", token);
                    insertBack(&fam->otherFields,field);
                }
                customFgets(line, 256, inFile, error);
                lineNumb++;
                if(error->type != OK){
                    error->type = INV_RECORD;
                    error->line = lineNumb;
                    deleteFamily(fam);
                    fclose(inFile);
                    return;
                }
                contconcCheck(line, inFile, &lineNumb, error);
            }
            insertBack(&temp->families, fam);
        }
    }
    //if parsed families successfully return OK
    fclose(inFile);
    error->type = OK;
}


void contconcCheck(char* line, FILE* inFile, int* lineNumb, GEDCOMerror* error){
    char temp[256];
    char temp2[256];
    char* token;

    error->type = OK;

    if(feof(inFile)){
        return;
    }

    customFgets(temp, 256, inFile, error);
    *lineNumb = *lineNumb + 1;
    strcpy(temp2, temp);

    strtok(temp2, " ");
    token = strtok(NULL, " ");
    
    if(token == NULL){
        fseek(inFile, -strlen(temp)-1, SEEK_CUR);
        *lineNumb = *lineNumb - 1;
        return;
    }

    //check if line continued on next line
    if(strncmp(token, "CONT", 4) == 0){
        token = strtok(NULL, "\0");
        strcat(line, "\n");
        strcat(line, token);
        contconcCheck(line, inFile, lineNumb, error);
    }
    //check if line concated on next line
    else if(strncmp(token, "CONC", 4) == 0){
        token = strtok(NULL, "\0");
        strcat(line, token);
        contconcCheck(line, inFile, lineNumb, error);
    }
    //otherwise seek back and perform operations
    else{
        fseek(inFile, -strlen(temp)-1, SEEK_CUR);
        *lineNumb = *lineNumb - 1;
    }

}

bool customFgets(char* line, int len, FILE* inFile, GEDCOMerror* error){
    memset(line, 0, len);
    size_t cur_len = strlen(line);
    char chr;

    while(1){
    if(feof(inFile)){
        error->type = OTHER_ERROR;
        return false;
    }
    chr = fgetc(inFile);
    cur_len = strlen(line);
    //check for valid line terminators
    if(chr == '\r' || chr == '\n'){
        if(strlen(line)==0){
            customFgets(line, 256, inFile, error);
        }
        chr = '\0';
        sprintf(line, "%s%c", line, chr);
        error->type = OK;
        return true;
    }
    //check if line is appropriate length
    else if(cur_len == len-1) {
        error->type = OTHER_ERROR;
        return false;
    }
        sprintf(line, "%s%c", line, chr);
        if(feof(inFile) && strncmp(line, "0 TRLR", 6) == 0){
            strcpy(line, "0 TRLR\0");
            error->type = OK;
            return true;
        }
    } 

}

bool compareTag(const void* a,const void* b) {
    //compare two individual tags for equality
    char* stringa = ((tagIndi*)a)->tag;
    char* stringb = (char*)b;
    if(strcmp(stringa, stringb) == 0){
        return true;
    }
    else{
        return false;
    }
}

char* tokenize (char line[]){
    char* token;
    //tokenize line 3 times to get value
    token = strtok(line, " ");
    token = strtok(NULL, " ");
    token = strtok(NULL, "");

    return token;
}

Submitter* createSubmitter(char* fileName, GEDCOMerror* error, char* subtag){
    FILE* inFile = fopen(fileName, "r");
    char* token;
    char* tag;
    char line[256];
    int lineNumb = 0;

    while (1){
        customFgets(line, 256, inFile, error);
        //look thru file for submitter tag
        if(strncmp(line,"0 TRLR", 6) == 0){
            error->type = INV_GEDCOM;
            error->line = -1;
            fclose(inFile);
            return NULL;
        }
        lineNumb++;
        token = strtok(line, " ");
        tag = strtok(NULL, " ");
        token = strtok(NULL, "");

        if(token == NULL){
            continue;
        }

        if(strcmp(subtag, tag) == 0 && strncmp(token, "SUBM", 4) == 0){
            break;
        }
        if(feof(inFile)){
            error->type = INV_RECORD;
            error->line = lineNumb;
            fclose(inFile);
            return NULL;
        }
    }

    //if submitter tag found create submitter
    Submitter* a = malloc(sizeof(Submitter) + sizeof(char) * 255);
    strcpy(a->address, "\0");
    a->otherFields = initializeList(&printField, &deleteField, &compareFields);

    while(1){
        // go thru submitter record and get fields
        customFgets(line, 256, inFile, error);
        lineNumb++;
        if(error->type != OK){
            fclose(inFile);
            clearList(&a->otherFields);
            free(a);
            error->type = INV_GEDCOM;
            error->line = -1;
            return NULL;
        }
        contconcCheck(line, inFile, &lineNumb, error);
        if(line[0] == '0'){
            break;
        }
        //check if submitter name line
        if (strncmp(line,"1 NAME",6) == 0)
        {
            token = tokenize(line);
            if(token == NULL){
                fclose(inFile);
                clearList(&a->otherFields);
                free(a);
                error->type = INV_RECORD;
                error->line = lineNumb;
                return NULL;
            }
            sprintf(a->submitterName, "%s", token);
        }
        //check if submitter adress record
        else if(strncmp(line,"1 ADDR",6) == 0){
            token = tokenize(line);
            if(token == NULL){
                fclose(inFile);
                clearList(&a->otherFields);
                free(a);
                error->type = INV_RECORD;
                error->line = lineNumb;
                return NULL;
            }
            sprintf(a->address, "%s", token);
        }
        //otherwise check if valid submitter field
        else{
            Field* field = malloc(sizeof(Field));
            token = strtok(line, " ");
            token = strtok(NULL, " ");
            if(token == NULL){
                fclose(inFile);
                clearList(&a->otherFields);
                free(a);
                error->type = INV_RECORD;
                error->line = lineNumb;
                return NULL;
            }
            field->tag = malloc(sizeof(char)* (strlen(token) + 1));
            sprintf(field->tag, "%s", token);
            token = strtok(NULL, "");
            if(token == NULL){
                fclose(inFile);
                clearList(&a->otherFields);
                free(a);
                error->type = INV_RECORD;
                error->line = lineNumb;
                return NULL;
            }
            field->value = malloc(sizeof(char)* (strlen(token) + 1));
            sprintf(field->value, "%s", token);
            insertBack(&a->otherFields,field);
        }
    }

    error->type = OK;

    fclose(inFile);
    return a;
}

char* printName(Individual* toBePrinted){

    if(toBePrinted == NULL){
        return "";
    }
    //malloc space for name
    char* toReturn = malloc(sizeof(char)*52);
    
    //copy name and return printable name
    strcpy(toReturn, "Name: ");
    strcat(toReturn, toBePrinted->givenName);
    strcat(toReturn, (" "));
    strcat(toReturn, toBePrinted->surname);
    strcat(toReturn, "\0");
    
    return toReturn;
}


void destroyNodeData(void *data){
    free((tagIndi*)data);
}

void dummyDelete(void *data){
}

bool findTag(const void* first,const void* second){
    Field* field = (Field*)first;
    char* string = (char*)second;

    if(strcmp(field->tag,string) == 0){
        return true;
    }

    return false;
}

void recursiveDescendant(List *descendants, Family *family, unsigned int maxGen, int count){

        Individual *individual;
        ListIterator iter = createIterator(family->children);

        while(iter.current != NULL){
            individual = (Individual*)iter.current->data;
            Individual *copy = copyIndi(individual);

            if(maxGen == 0 || count <= maxGen){
                if(descendants->length < count){
                    List *generation = malloc(sizeof(List));
                    *generation = initializeList(&printIndividual, &deleteIndividual, &compareSurname);
                    insertBack(generation, copy);
                    insertBack(descendants, generation);
                } 
                else {
                    ListIterator iter = createIterator(*descendants);
                    List *generation;
                    for (int i = 1; i < count; ++i){
                        nextElement(&iter);
                    }
                    generation = (List*)iter.current->data;
                    insertSorted(generation, copy);
                }
                Family *family;
                ListIterator iter = createIterator(individual->families);
                while(iter.current != NULL){
                    family = (Family*)iter.current->data;
                    if(family->wife == individual || family->husband == individual){
                        recursiveDescendant(descendants, family, maxGen, count + 1);
                    }
                    nextElement(&iter);
                }
            }
            nextElement(&iter);
        }

        return;
}

void recursiveAnscestor(List *descendants, Family *family, unsigned int maxGen, int count){

    Individual *individual = family->husband;

    if(individual != NULL){
        Individual *copy = copyIndi(individual);

        if(maxGen == 0 || count <= maxGen){
            if(descendants->length < count){
                List *generation = malloc(sizeof(List));
                *generation = initializeList(&printIndividual, &deleteIndividual, &compareSurname);
                insertBack(generation, copy);
                insertBack(descendants, generation);
            } 
            else {
                ListIterator iter = createIterator(*descendants);
                List *generation;
                for (int i = 1; i < count; ++i){
                    nextElement(&iter);
                }

                generation = (List*)iter.current->data;
                insertSorted(generation, copy);
            }

            Family *family;
            ListIterator iter = createIterator(individual->families);
            while(iter.current != NULL){
                family = (Family*)iter.current->data;
                if(family->wife != individual && family->husband != individual){
                    recursiveAnscestor(descendants, family, maxGen, count+1);
                }
                nextElement(&iter);
            }
        } 

    }


    individual = family->wife;

    if(individual != NULL){
        Individual *copy = copyIndi(individual);

        if(maxGen == 0 || count <= maxGen){
            if(descendants->length < count){
                List *generation = malloc(sizeof(List));
                *generation = initializeList(&printIndividual, &deleteIndividual, &compareSurname);
                insertBack(generation, copy);
                insertBack(descendants, generation);         
            } 
            else {
                ListIterator iter = createIterator(*descendants);
                List *generation;
                for (int i = 1; i < count; ++i){
                    nextElement(&iter);
                }

                generation = (List*)iter.current->data;
                insertSorted(generation, copy);
            }
                
            Family *family;
            ListIterator iter = createIterator(individual->families);
            while(iter.current != NULL){
                family = (Family*)iter.current->data;
                if(family->wife != individual && family->husband != individual){
                    recursiveAnscestor(descendants, family, maxGen, count+1);
                }
                nextElement(&iter);
            }
        } 
    }

    return;
}

int compareSurname(const void* a,const void* b){
    Individual* first = (Individual*)a;
    Individual* second = (Individual*)b;

    if(strcmp(first->surname,second->surname) != 0){
        return strcmp(first->surname,second->surname);
    }

    return strcmp(first->givenName,second->givenName);
}

bool findFamily(const void* a,const void* b){
    Family* first = ((storeFam*)a)->temp;
    Family* second = (Family*)b;

    if(first->husband == NULL && second->husband == NULL && 
        first->wife == NULL && second->wife == NULL &&
        first->children.length == second->children.length){
        return true;
    }
    else if(first->husband == NULL && second->husband == NULL && 
        compareIndividuals(first->wife, second->wife) == 0 &&
        first->children.length == second->children.length){
        return true;
    }
    else if(compareIndividuals(first->husband, second->husband) == 0 && 
        first->wife == NULL && second->wife == NULL &&
        first->children.length == second->children.length){
        return true;
    }
    else if(compareIndividuals(first->husband, second->husband) == 0 && 
        compareIndividuals(first->wife, second->wife) == 0 &&
        first->children.length == second->children.length){
        return true;
    }

    return false;
}

bool findIndividual(const void* a,const void* b){
    Individual* first = ((storeIndi*)a)->temp;
    Individual* second = (Individual*)b;

    if(compareIndividuals(first, second) == 0 && cmpEvent(first,second)){
        return true;
    }

    return false;
}

bool findIndi(const void* a,const void* b){
    Individual* first = (Individual*)a;
    Individual* second = (Individual*)b;

    if(compareIndividuals(first, second) == 0 && cmpEvent(first,second)){
        return true;
    }

    return false;
}

