#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "hashStat.h"

int main()
{
  Hashtable h;
  int size;
  double **a;
  char **stringKeyList;
  char ***vectorKeyList;
  int *dimensionList;
  double e, v;
  int i, j;

  Hash_init(&h, 7);
  
  Hash_InsertKey(&h, "2");
  Hash_InsertKey_multiD(&h, 2, "200", "$256");
  Hash_InsertKey_multiD(&h, 3, "abc", "def", "ghi");
  Hash_InsertKey_multiD(&h, 2, "abc", "def");
  Hash_InsertKey_multiD(&h, 5, "@", "*(^B", "#(#D", "l", "F R3e");
  Hash_InsertKey(&h, "$244d");

  Hash_RemoveKey(&h, "2");
  Hash_RemoveKey(&h, "abc");
  Hash_RemoveKey_multiD(&h, 2, "abc", "def");

  Hash_AddData(&h, 4, "2");
  Hash_AddData(&h, 3.5, "2");
  Hash_AddData(&h, 6.22, "2");
  Hash_AddData_multiD(&h, 2, 2, "200", "$256");
  Hash_AddData_multiD(&h, 5.5, 2, "abc", "def");
  Hash_AddData_multiD(&h, 4.5, 2, "abc", "def");
  Hash_AddData_multiD(&h, 7, 1, "$244d");
  Hash_AddData(&h, 6.7, "$244d");
  Hash_AddData_multiD(&h, 6.7, 2, "$244d", "zzz");
  Hash_AddData_multiD(&h, 398, 1, "zzz_fwc");
  Hash_AddData(&h, 104.355, "33");
  Hash_AddData_multiD(&h, 4.56, 1, "33");

  Hash_GetSize(&h, &size);

  printf("\nOUTPUT TESTING RESULTS\n");
  Hash_OutputData(&h, &e, &v, "2");
  printf("e=%f\tv=%f\n", e, sqrt(v));
  Hash_OutputData(&h, &e, &v, "$244d");
  printf("e=%f\tv=%f\n", e, sqrt(v));
  Hash_OutputData_multiD(&h, &e, &v, 2, "3", "3");
  Hash_OutputData(&h, &e, &v, "abcdefghi");
  Hash_OutputData_multiD(&h, &e, &v, 1, "33");
  printf("e=%f\tv=%f\n", e, sqrt(v));
  Hash_OutputData(&h, &e, &v, "fwefew");

  printf("\nDUMP TESTING RESULTS");
  printf("\nKEY                               MEAN      STD DEV\n");
  stringKeyList = (char **)malloc(size * sizeof(char *));
  vectorKeyList = (char ***)malloc(size * sizeof(char **));
  dimensionList = (int *)malloc(size * sizeof(int));
  a = (double **)malloc(size * sizeof(double *));
  for(i=0; i<size; i++)
    a[i] = (double *)malloc(2 * sizeof(double));

  Hash_Dump(&h, a, stringKeyList);

  for(i=0; i< size; i++)
    printf("%-25s%13.6f%13.6f\n", stringKeyList[i], a[i][0], sqrt(a[i][1]));

  printf("\nMULTID DUMP TESTING RESULTS");
  printf("\nKEY              DIMENSION         MEAN      STD DEV\n");
  Hash_Dump_multiD(&h, a, stringKeyList, dimensionList, vectorKeyList);
  
  for(i=0; i< size; i++)
  {
    printf("%-25s%d%13.6f%13.6f\n", stringKeyList[i], dimensionList[i],
        a[i][0], sqrt(a[i][1]));
    printf("           key vector:     ");
    for(j=0; j<dimensionList[i]; j++)
      printf("%-5s  ", vectorKeyList[i][j]);
    printf("\n");
  }
  
  Hash_del(&h);
  
  free(stringKeyList);
  free(vectorKeyList);
  free(dimensionList);
  for(i=0; i<size; i++)
    free(a[i]);
  free(a);
}
