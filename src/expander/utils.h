/**
 * XtsTiExpander V2
 * 
 * Utils
 * 
 * Xtase - fgalliat @Aug 2021
 */

typedef struct LList {
  char* value;
  LList* prev = NULL;
  LList* next = NULL;
} LList;

LList* createListEntry(char* value, LList* prev=NULL);
LList* l_getFirst(LList* entry);
char** l_toArray(LList* entry, int& retSize);
void l_free(LList* entry);

void a_quickSort(char* tableau[], int debut, int fin);
