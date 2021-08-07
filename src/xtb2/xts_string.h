#ifndef _XTS_STRING_H_
#define _XTS_STRING_H_ 1

/**
 * Xtase - fgalliat @Jan2019
 * str functions IMPL
 */

// always free-able
char* makeString(int len);

// makes a safe copy
char* copyOf(char* str, int len=-1);

// makes a safe copy
char* substring(char* str, int start, int stop=-1);

bool contains(char* str, char* toFind);

int indexOf(char* str, char ch, int start=0);


char* str_split(char* stringToSplit, char delim, int tokenNum);

int str_count(char* stringToSplit, char delim);

char* upper(char* str);

char* lower(char* str);

bool endsWith(char* str, char* toFind);

bool startsWith(char* str, char* toFind);

bool equals(char* str, char* toFind);

char charUpCase(char ch);


#endif