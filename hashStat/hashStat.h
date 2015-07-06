#ifndef HASHSTAT_H__ 
#define HASHSTAT_H__ 

#include <stdarg.h>
  
typedef struct hashTable_t hashTable;

hashTable *Hash_init(int m);
void Hash_del(hashTable *h);
void Hash_getSize(hashTable *h, int *size);
void Hash_insertKey(hashTable *h, const char *key); 
void Hash_removeKey(hashTable *h, const char *key);
void Hash_addData(hashTable *h, double x, const char *key);
void Hash_outputData(hashTable *h, double *e, double *v, const char *key);
void Hash_dump(hashTable *h, double **arr, char **keyList);

/* use these functions if you want a multi-dimensional key. */
void Hash_insertKey_multiD(hashTable *h, int d, ...);
void Hash_removeKey_multiD(hashTable *h, int d, ...);
void Hash_addData_multiD(hashTable *h, double x, int d, ...);
void Hash_outputData_multiD(hashTable *h, double *e, double *v, int d, ...);
void Hash_dump_multiD(hashTable *h, double **arr, char **keyList, 
    int *dimensionList, char ***vectorKeyList);

#endif
