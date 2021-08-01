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
    v0->prev = prev;
    v0->next = NULL;
    if ( prev != NULL ) {
        prev->next = v0;
    }
    return v0;
}

void l_print(LList* entry) {
    printf("%s -> [%s] -> %s \n", (entry->prev == NULL ? "NULL":entry->prev->value), entry->value, (entry->next == NULL ? "NULL":entry->next->value));
}

void listEntries(LList* entry) {
    if ( entry == NULL ) { return; }
    // printf("%s \n", entry->value);
    l_print( entry );
    listEntries(entry->next);
}

// entry <= other
bool l_isLessThan(LList* entry, LList* other) {
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

bool l_remove(LList* entry) {
    if ( entry->prev == NULL && entry->next == NULL ) {
        return false;
    }

    if (entry->prev != NULL) {
        // printf("remove from prev\n");
        entry->prev->next = entry->next;
    }

    if ( entry->next != NULL) {
        // printf("remove from next\n");
        entry->next->prev = entry->prev;
    }

    entry->prev = NULL;
    entry->next = NULL;
    return true;
}


bool l_insertBefore(LList* entry, LList* beforeThat) {
  if ( entry->next == beforeThat ) {
      return false;
  }
  l_remove( entry );
  entry->next = beforeThat;

  if ( beforeThat != NULL ) { 
      entry->prev = beforeThat->prev;
      if ( entry->prev != NULL ) {
          entry->prev->next = entry;
      }
      beforeThat->prev = entry; 
  }
  return true;
}

// void l_swap(LList* entry, LList* beforeThat) {

// }

void l_sort(LList* entry) {
    // FIXME
    LList* next = entry->next;
    int size = l_size( entry );
    for(int i=0; i < size; i++) {
        if ( next == NULL ) { break; }
        while( l_isLessThan( next, next->prev ) ) {
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

    // LList* v0 = createListEntry((char*)"menu.12");
    // LList* v1 = createListEntry((char*)"moondat.1C", v0);
    // LList* v2 = createListEntry((char*)"moon.C", v1);
    // LList* v3 = createListEntry((char*)"abc.12", v2);

    LList* v0 = createListEntry((char*)"v0");
    LList* v1 = createListEntry((char*)"v1", v0);
    LList* v2 = createListEntry((char*)"v2", v1);
    LList* v3 = createListEntry((char*)"v3", v2);


    printf("*-========-* \n");
    listEntries(v0);

    printf("Size : %d \n", l_size(v0));

    //l_sort(v0);

    l_insertBefore(v3, v0); // works
    l_insertBefore(v2, v1); // works
    // l_remove(v0);

    // printf("*-========-* \n");
    // listEntries(v0);

    printf("*-========-* \n");
    listEntries(l_getFirst(v3));
    // listEntries(v1);

    return 0;
}