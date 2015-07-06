#ifndef HASHSTAT_H__ 
#define HASHSTAT_H__ 

#include <stdarg.h>

typedef struct HashNode_t HashNode;
struct HashNode_t{
  int dimension;
  char **vectorKey;
  char *stringKey;
  double s, e, v;
  HashNode *next;
};
  
typedef struct Hashtable_t{
  int m; /* mod */
  int size;
  HashNode **table;
}Hashtable;

void Hash_init(Hashtable *h, int m);
void Hash_del(Hashtable *h);
void Hash_GetSize(Hashtable *h, int *size);
void Hash_InsertKey(Hashtable *h, const char *key); 
void Hash_RemoveKey(Hashtable *h, const char *key);
void Hash_AddData(Hashtable *h, double x, const char *key);
void Hash_OutputData(Hashtable *h, double *e, double *v, const char *key);
void Hash_Dump(Hashtable *h, double **arr, char **keyList);

/* C does not support overloading :(    */
void Hash_InsertKey_multiD(Hashtable *h, int d, ...);
void Hash_RemoveKey_multiD(Hashtable *h, int d, ...);
void Hash_AddData_multiD(Hashtable *h, double x, int d, ...);
void Hash_OutputData_multiD(Hashtable *h, double *e, double *v, int d, ...);
void Hash_Dump_multiD(Hashtable *h, double **arr, char **keyList, 
    int *dimensionList, char ***vectorKeyList);

#endif
