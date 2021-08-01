/**
 * XtsTiExpander V2
 * 
 * Utils
 * 
 * Xtase - fgalliat @Aug 2021
 */

#include <Arduino.h>
#include "utils.h"

LList* createListEntry(char* value, LList* prev/*=NULL*/) {
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
    Serial.printf("%s -> [%s] -> %s \n", (entry->prev == NULL ? "NULL":entry->prev->value), entry->value, (entry->next == NULL ? "NULL":entry->next->value));
}

void listEntries(LList* entry) {
    if ( entry == NULL ) { return; }
    // printf("%s \n", entry->value);
    l_print( entry );
    listEntries(entry->next);
}

// entry < other
bool s_isLessThan(char* entry, char* other) {
    if ( other == NULL ) { return false; }
    // int minLen = min( strlen(entry), strlen(other) );
    // for(int i=0; i < minLen; i++) {
    //     if ( entry[i] < other[i] ) {
    //         continue;
    //     }
    //     return false;
    // }
    return strcmp(entry, other) < 0;
    return true;
}

// entry < other
bool l_isLessThan(LList* entry, LList* other) {
    if ( other == NULL ) { return false; }
    return s_isLessThan(entry->value, other->value);
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

void l_swap(LList* entry, LList* other) {
    LList* swPrev = entry->prev;
    LList* swNext = entry->next;
    entry->prev = other->prev;
    if ( entry->prev != NULL ) { entry->prev->next = entry; }
    entry->next = other->next;
    if ( entry->next != NULL ) { entry->next->prev = entry; }
    other->prev = swPrev;
    if ( other->prev != NULL ) { other->prev->next = other; }
    other->next = swNext;
    if ( other->next != NULL ) { other->next->prev = other; }
}

void l_sort(LList* entry) {
    // // FIXME
    // LList* next = entry->next;
    // int size = l_size( entry );
    // for(int i=0; i < size; i++) {
    //     if ( next == NULL ) { break; }
    //     while( l_isLessThan( next, next->prev ) ) {
    //         l_insertBefore(next, next->prev);
    //     }
    //     next = next->next;
    // }
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

LList* l_getLast(LList* entry) {
    if ( entry == NULL ) { return NULL; }
    if ( entry->next == NULL ) { return entry; }
    LList* cur = entry;
    while (cur->next != NULL) {
        cur = cur->next;
    }
    return cur;
}

void l_free(LList* entry) {
    if ( entry == NULL ) { return; }
    LList* cur = l_getLast(entry);
    while (cur != NULL) {
        LList* p = cur->prev;
        free(cur);
        cur = p;
    }
}

char** l_toArray(LList* entry, int& retSize) {
    if ( entry == NULL ) { return NULL; }
    LList* cur = entry;
    int size = l_size(entry);
    char** ret = (char**)malloc( size * sizeof(char*) );
    for(int i=0; i < size; i++) {
        ret[i] = cur->value;
        cur = cur->next;
    }
    retSize = size;
    return ret;
}

// ====================
void a_swap(char* tableau[], int a, int b) {
    char* temp = tableau[a];
    tableau[a] = tableau[b];
    tableau[b] = temp;
}

void a_quickSort(char* tableau[], int debut, int fin) {
    int gauche = debut-1;
    int droite = fin+1;
    char* pivot = tableau[debut];
    // printf("pivot[%s]\n", pivot);

    /* Si le tableau est de longueur nulle, il n'y a rien à faire. */
    if(debut >= fin)
        return;

    /* Sinon, on parcourt le tableau, une fois de droite à gauche, et une
       autre de gauche à droite, à la recherche d'éléments mal placés,
       que l'on permute. Si les deux parcours se croisent, on arrête. */
    while(1) {
        // do droite--; while(tableau[droite] > pivot);
        // do gauche++; while(tableau[gauche] < pivot);
        do droite--; while(s_isLessThan( pivot, tableau[droite] ));
        do gauche++; while(s_isLessThan( tableau[gauche], pivot ));

        if(gauche < droite)
            a_swap(tableau, gauche, droite);
        else break;
    }

    // printf("droite[%d]\n", droite);

    /* Maintenant, tous les éléments inférieurs au pivot sont avant ceux
       supérieurs au pivot. On a donc deux groupes de cases à trier. On utilise
       pour cela... la méthode quickSort elle-même ! */
    a_quickSort(tableau, debut, droite);
    a_quickSort(tableau, droite+1, fin);
}
