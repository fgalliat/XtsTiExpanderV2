#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define min(a,b) ( a < b ? a : b )

typedef struct LList {
  char* value;
  LList* prev = NULL;
  LList* next = NULL;
} LList;

LList* createListEntry(char* value, LList* prev=NULL) {
    LList* v0 = (LList*)malloc( 1 * sizeof( LList ) );
    v0->value = (char*)value;
    v0->prev=prev;
    if ( prev != NULL ) {
        prev->next = v0;
    }
    return v0;
}

void listEntries(LList* entry) {
    if ( entry == NULL ) { return; }
    printf("%s \n", entry->value);
    listEntries(entry->next);
}

// entry < other
bool l_isBefore(LList* entry, LList* other) {
    if ( other == NULL ) { return false; }
    int minLen = min( strlen(entry->value), strlen(other->value) );
    for(int i=0; i < minLen; i++) {
        if ( other->value[i] < entry->value[i] ) {
            return false;
        }
    }
    return true;
}

int l_size(LList* entry, int cpt=0) {
  if ( entry == NULL ) { return cpt; }
  cpt++;
  return l_size( entry->next, cpt );
}

void l_insertBefore(LList* entry, LList* beforeThat) {
  // FIXME : check that code ...
  if ( entry->prev != NULL ) {
      entry->prev->next = entry->next;
  }
  if ( entry->next != NULL ) {
      //if ( entry->prev != NULL ) {
          entry->next->prev = entry->prev;
      //}
  }
  entry->next = beforeThat;
  if ( beforeThat != NULL ) { beforeThat->next = entry; }
}

void l_sort(LList* entry) {
    // FIXME
    LList* next = entry->next;
    int size = l_size( entry );
    for(int i=0; i < size; i++) {
        if ( next == NULL ) { break; }
        while( l_isBefore( next, next->prev ) ) {
            l_insertBefore(next, next->prev);
        }
        next = next->next;
    }
}

LList* l_getFirst(LList* entry) {
    if ( entry == NULL ) { return NULL; }
    if ( entry->prev == NULL ) { return entry; }
    LList* cur = entry;
    while (cur->prev != NULL) {
        cur = cur->prev;
    }
    return cur;
}

// =====================================================================

int main(int argc, char** argv) {
    printf("Coucou \n");

    LList* v0 = createListEntry((char*)"menu.12");
    LList* v1 = createListEntry((char*)"moondat.1C", v0);
    LList* v2 = createListEntry((char*)"moon.C", v1);
    LList* v3 = createListEntry((char*)"abc.12", v2);

    printf("*-========-* \n");
    listEntries(v0);

    printf("Size : %d \n", l_size(v0));

    l_sort(v0);
    printf("*-========-* \n");
    listEntries(l_getFirst(v0));

    return 0;
}