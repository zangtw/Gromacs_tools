#ifndef HASHSTAT_H__ 
#define HASHSTAT_H__ 

#include <stdarg.h>
  
typedef struct hashTable_t hashTable;

hashTable *Hash_init(int m);
void Hash_del(hashTable *h);
void Hash_getSize(hashTable *h, int *size);
void Hash_insertKey(hashTable *h, const char *format, ...);
void Hash_removeKey(hashTable *h, const char *format, ...);
void Hash_addData(hashTable *h, double x, const char *format, ...);
void Hash_printData(hashTable *h, double *e, double *v, 
    const char *format, ...);
void Hash_dump(hashTable *h, double **arr, char **keyList, 
    int *dimensionList, char **formatList, void **vectorKeyList);

#define MAX_KEY_LENGTH 256

#endif
