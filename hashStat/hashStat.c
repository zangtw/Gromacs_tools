#include "hashStat.h"
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

typedef struct hashNode_t hashNode;
struct hashNode_t{
  int dimension;
  char **vectorKey;
  char *stringKey;
  double s, e, v;
  hashNode *next;
};

struct hashTable_t{
  int m; /* mod */
  int size;
  hashNode **table;
};

static int hash(const char *str, int m)
{
  unsigned long hash = 5381;
  int i = 0;

  while (i < strlen(str))
  {
    hash = ((hash << 5) + hash) + str[i];
    i++;
  }

  return hash % m;
}

hashTable *Hash_init(int m)
{
  hashTable *h = (hashTable *)malloc(sizeof(hashTable));

  h->m = m;
  h->size = 0;
  h->table = (hashNode **)malloc(m * sizeof(hashNode *));

  int i;
  for(i=0; i<m; i++)
    h->table[i] = NULL;

  return h;
}

void Hash_del(hashTable *h)
{
  int i, j;
  hashNode *currNode, *nextNode;

  for(i=0; i< h->m; i++)
  {
    currNode = h->table[i];

    while(currNode != NULL)
    {
      nextNode = currNode->next;
      
      for(j=0; j<currNode->dimension; j++)
        free(currNode->vectorKey[j]);
      free(currNode->vectorKey);
      free(currNode->stringKey);
      free(currNode);
      h->size--;
      currNode = nextNode;
    }
  }

  free(h->table);
  free(h);
}

void Hash_getSize(hashTable *h, int *size)
{
  *size = h->size;
}

static void genMultiDimensionalStrings(int d, char **v, char *s, va_list args)
{
  int i;
  char *temp;

  if(s != NULL) { s[0] = '|'; s[1] = '\0';}

  for(i=0; i<d; i++)
  {
    temp = va_arg(args, char*);
    
    if(s != NULL)
      sprintf(s+strlen(s), "%s|", temp);
    
    if(v != NULL)
      v[i] = temp;
  }
}

static void Hash_insertKey_Kernel(hashTable *h, char *key, int d, va_list args)
{
  int bBuf = (key == NULL); 
  char string_buf[256]; /* will only be used when key is not available. */
  char **vector_buf;
  int i;
  int k;

  vector_buf = (char **)malloc(d * sizeof(char *));
  genMultiDimensionalStrings(d, vector_buf, bBuf ? string_buf : NULL, args);
  
#define mykey (bBuf ? string_buf : key)

  k = hash(mykey, h->m);
  
  hashNode *currNode = h->table[k];
  hashNode *newNode;

  while(currNode != NULL)
  {
    if(! strcmp(mykey, currNode->stringKey))
    {
      fprintf(stderr, "key already exists!\n");
      return;
    }
    currNode = currNode->next;
  }

  newNode = (hashNode *)malloc(sizeof(hashNode));
  h->size++;

  newNode->dimension = d;
  newNode->vectorKey = (char **)malloc(d * sizeof(char *));
  for(i=0; i<d; i++)
  {
    newNode->vectorKey[i] = (char *)malloc(256 * sizeof(char));
    newNode->vectorKey[i][0] = '\0';  /* is it necessary?? */
    strcat(newNode->vectorKey[i], vector_buf[i]); /* copy from the stack */
  }
  newNode->stringKey = (char *)malloc(256 * sizeof(char));
  newNode->stringKey[0] = '\0'; 
  strcat(newNode->stringKey, mykey);
  newNode->s = newNode->e = newNode->v = 0;

  newNode->next = h->table[k];
  h->table[k] = newNode;

  free(vector_buf);

#undef mykey
}

void Hash_insertKey_multiD(hashTable *h, int d, ...)
{
  va_list args;
  va_start(args, d);
  
  Hash_insertKey_Kernel(h, NULL, d, args);

  va_end(args);
}

void Hash_removeKey_multiD(hashTable *h, int d, ...)
{
  va_list args;
  char buf[256];
  int i;
  int k;
  
  va_start(args, d);
  genMultiDimensionalStrings(d, NULL, buf, args);
  va_end(args);
  
  k = hash(buf, h->m);
  hashNode *currNode = h->table[k];
  hashNode *lastNode;

  lastNode = currNode;
  while(currNode != NULL)
  {
    if(! strcmp(buf, currNode->stringKey))
    {
      if (lastNode == currNode) /* indicating currNode is the first node */
        h->table[k] = currNode->next;
      else
        lastNode->next = currNode->next;
      
      for(i=0; i<currNode->dimension; i++)
        free(currNode->vectorKey[i]);
      free(currNode->vectorKey);
      free(currNode->stringKey);
      free(currNode);
      h->size--;

      return;
    }

    lastNode = currNode;
    currNode = currNode->next;
  }
      
  fprintf(stderr, "RemoveKey: key \"%s\" does not exist! \
@%s, line %d.\n", buf, __FILE__, __LINE__);
}

void Hash_addData_multiD(hashTable *h, double x, int d, ...)
{
  va_list args;
  char buf[256];
  int k;
  
  va_start(args, d);
  genMultiDimensionalStrings(d, NULL, buf, args);
  va_end(args);
  
  k = hash(buf, h->m);
  hashNode *currNode = h->table[k];

  while(currNode != NULL)
  {
    if(! strcmp(buf, currNode->stringKey))
    {
      double s_old = currNode->s;
      double e_old = currNode->e;
      currNode->s += 1;
      currNode->e += x;
      if(s_old != 0)
        currNode->v += (x - currNode->e / currNode->s) * (x - e_old / s_old);
      
      return;
    }

    currNode = currNode->next;
  }

  fprintf(stderr, "AddData: key \"%s\" is not available. Will add a new key. \
@%s, line %d.\n", buf, __FILE__, __LINE__);

  va_start(args, d);
  Hash_insertKey_Kernel(h, buf, d, args);
  va_end(args);
  
  currNode = h->table[k];
  currNode->s += 1;
  currNode->e += x;
}

void Hash_outputData_multiD(hashTable *h, double *e, double *v, int d, ...)
{
  va_list args;
  char buf[256];
  int k;
  
  va_start(args, d);
  genMultiDimensionalStrings(d, NULL, buf, args);
  va_end(args);
  
  k = hash(buf, h->m);
  hashNode *currNode = h->table[k];

  while(currNode != NULL)
  {
    if(! strcmp(buf, currNode->stringKey))
    {
      if(currNode->s == 0)
      {
        *e = 0;
        *v = 0;
      }
      else
      {
        *e = currNode->e / currNode->s;
        *v = currNode->v / currNode->s;
      }
      return;
    }

    currNode = currNode->next;
  }
  
  fprintf(stderr, "OutputData: key \"%s\" does not exist! \
@%s, line %d.\n", buf, __FILE__, __LINE__);
}

void Hash_dump_multiD(hashTable *h, double **arr, char **keyList, 
    int *dimensionList, char ***vectorKeyList)
{
  int i, j;
  char buf[256];
  hashNode *currNode;

  for(i=0, j=0; i<h->m; i++)
  {
    currNode = h->table[i];

    while(currNode != NULL)
    {
      if(keyList != NULL)
        keyList[j] = currNode->stringKey;

      if(dimensionList != NULL)
        dimensionList[j] = currNode->dimension;

      if(vectorKeyList != NULL)
        vectorKeyList[j] = currNode->vectorKey;
      
      if(arr != NULL)
      {
        if(arr[j] != NULL)
        {
          buf[0] = '\0';
          strcat(buf, currNode->stringKey);
          buf[strlen(buf)-1] = '\0';
          Hash_outputData_multiD(h, &arr[j][0], &arr[j][1], 1, buf+1);
        }
        else
        {
          fprintf(stderr, 
              "The size of input array (%d) does not match the size of hash table (%d) !", 
              j, h->size);
          return;
        }
      }
      
      currNode = currNode->next;
      j++;
    }
  }
}

/* C does not support overloading :(    */
void Hash_insertKey(hashTable *h, const char *key) 
{ Hash_insertKey_multiD(h, 1, key); }
void Hash_removeKey(hashTable *h, const char *key)
{ Hash_removeKey_multiD(h, 1, key); }
void Hash_addData(hashTable *h, double x, const char *key)
{ Hash_addData_multiD(h, x, 1, key); }
void Hash_outputData(hashTable *h, double *e, double *v, const char *key)
{ Hash_outputData_multiD(h, e, v, 1, key); }
void Hash_dump(hashTable *h, double **arr, char **keyList)
{ Hash_dump_multiD(h, arr, keyList, NULL, NULL); }
