#include "hashStat.h"
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

typedef struct hashNode_t hashNode;
struct hashNode_t{
  int dimension;
  char *format;
  char *stringKey;
  void **vectorKey;
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

  while (*str++)
    hash = ((hash << 5) + hash) + *str;

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
      
      for(j=0; j< currNode->dimension; j++)
      {
        switch (currNode->format[j])
        {
          case 's':
            free((char*) (currNode->vectorKey[j])); break;
          case 'f':
            free((double *) (currNode->vectorKey[j])); break;
          default:
            free((int *) (currNode->vectorKey[j])); break;
        }
      }
      free(currNode->vectorKey);
      free(currNode->format);
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

static void genMultiDimensionalKeys(const char *format, void **v, 
    char *s, va_list args)
{
  char *s_temp;
  int d_temp;
  double f_temp;
  void **v_temp = v;

  if(s != NULL) { s[0] = '|'; s[1] = '\0';}

  while( *format != '\0' )
  {
    switch(*format)
    {
      case 's':
        s_temp = va_arg(args, char*);
        if(s != NULL)
          sprintf(s+strlen(s), "%s|", s_temp);
        if(v != NULL)
        {
          *v_temp = (void *)malloc((strlen(s_temp) + 1) * sizeof(char));
          strcpy((char *)(*v_temp), s_temp);
        }
        break;
      case 'f':
        f_temp = va_arg(args, double);
        if(s != NULL)
          sprintf(s+strlen(s), "%fF|", f_temp);
        if(v != NULL)
        {
          *v_temp = (double *)malloc(sizeof(double));
          *((double *)(*v_temp)) = f_temp;
        }
        break;
      default:  /* int */
        d_temp = va_arg(args, int);
        if(s != NULL)
          sprintf(s+strlen(s), "%dD|", d_temp);
        if(v != NULL)
        {
          *v_temp = (int *)malloc(sizeof(int));
          *((int *)(*v_temp)) = d_temp;
        }
    }
    
    if(v != NULL)
      v_temp ++;

    format++;
  }
}

static void Hash_insertKey_Kernel(hashTable *h, char *key, 
    const char *format, va_list args)
{
  int bBuf = (key == NULL); 
  char string_buf[MAX_KEY_LENGTH]; /* will only be used when key is not available. */
  void **vector_buf;
  int dimension = strlen(format);
  int i;
  int k;

  vector_buf = (void **)malloc(dimension * sizeof(void *));
  genMultiDimensionalKeys(format, vector_buf, bBuf ? string_buf : NULL, args);
  
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

  newNode->dimension = dimension;
  
  newNode->stringKey = (char *)malloc(MAX_KEY_LENGTH * sizeof(char));
  strcpy(newNode->stringKey, mykey);
  
  newNode->format = (char *)malloc((dimension + 1) * sizeof(char));
  strcpy(newNode->format, format);
  
  newNode->vectorKey = (void **)malloc(dimension * sizeof(void *));
  for(i=0; i<dimension; i++)
    newNode->vectorKey[i] = vector_buf[i];
  newNode->s = newNode->e = newNode->v = 0;

  newNode->next = h->table[k];
  h->table[k] = newNode;

  free(vector_buf);

#undef mykey
}

void Hash_insertKey(hashTable *h, const char *format, ...)
{
  va_list args;
  va_start(args, format);
  
  Hash_insertKey_Kernel(h, NULL, format, args);

  va_end(args);
}

void Hash_removeKey(hashTable *h, const char *format, ...)
{
  va_list args;
  char buf[MAX_KEY_LENGTH];
  int i;
  int k;
  
  va_start(args, format);
  genMultiDimensionalKeys(format, NULL, buf, args);
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
      
      for(i=0; i< currNode->dimension; i++)
      {
        switch (currNode->format[i])
        {
          case 's':
            free((char*) (currNode->vectorKey[i])); break;
          case 'f':
            free((double *) (currNode->vectorKey[i])); break;
          default:
            free((int *) (currNode->vectorKey[i])); break;
        }
      }
      free(currNode->vectorKey);
      free(currNode->stringKey);
      free(currNode->format);
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

void Hash_addData(hashTable *h, double x, const char *format, ...)
{
  va_list args;
  char buf[MAX_KEY_LENGTH];
  int k;
  
  va_start(args, format);
  genMultiDimensionalKeys(format, NULL, buf, args);
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

  va_start(args, format);
  Hash_insertKey_Kernel(h, buf, format, args);
  va_end(args);
  
  currNode = h->table[k];
  currNode->s += 1;
  currNode->e += x;
}

void Hash_printData(hashTable *h, double *e, double *v, 
    const char *format, ...)
{
  va_list args;
  char buf[MAX_KEY_LENGTH];
  int k;
  
  va_start(args, format);
  genMultiDimensionalKeys(format, NULL, buf, args);
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

void Hash_dump(hashTable *h, double **arr, char **keyList, 
    int *dimensionList, char **formatList, void ***vectorKeyList)
{
  int i, j;
  char buf[MAX_KEY_LENGTH];
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

      if(formatList != NULL)
        formatList[j] = currNode->format;

      if(vectorKeyList != NULL)
        vectorKeyList[j] = currNode->vectorKey;
      
      if(arr != NULL)
      {
        if(arr[j] != NULL)
        {
          strcpy(buf, currNode->stringKey);
          buf[strlen(buf)-1] = '\0';
          Hash_printData(h, &arr[j][0], &arr[j][1], "s", buf+1);
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
