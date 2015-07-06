#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "hashStat.h"

int main()
{
  int size;
  double **a;
  char **stringKeyList;
  char ***vectorKeyList;
  int *dimensionList;
  double e, v;
  int i, j;

  hashTable *h = Hash_init(7);
  
  Hash_insertKey(h, "2");
  Hash_insertKey_multiD(h, 2, "200", "$256");
  Hash_insertKey_multiD(h, 3, "abc", "def", "ghi");
  Hash_insertKey_multiD(h, 2, "abc", "def");
  Hash_insertKey_multiD(h, 5, "@", "*(^B", "#(#D", "l", "F R3e");
  Hash_insertKey(h, "$244d");

  Hash_removeKey(h, "2");
  Hash_removeKey(h, "abc");
  Hash_removeKey_multiD(h, 2, "abc", "def");

  Hash_addData(h, 4, "2");
  Hash_addData(h, 3.5, "2");
  Hash_addData(h, 6.22, "2");
  Hash_addData_multiD(h, 2, 2, "200", "$256");
  Hash_addData_multiD(h, 5.5, 2, "abc", "def");
  Hash_addData_multiD(h, 4.5, 2, "abc", "def");
  Hash_addData_multiD(h, 7, 1, "$244d");
  Hash_addData(h, 6.7, "$244d");
  Hash_addData_multiD(h, 6.7, 2, "$244d", "zzz");
  Hash_addData_multiD(h, 398, 1, "zzz_fwc");
  Hash_addData(h, 104.355, "33");
  Hash_addData_multiD(h, 4.56, 1, "33");

  Hash_getSize(h, &size);
  printf("\nsize=%d\n",size);

  printf("\nOUTPUT TESTING RESULTS\n");
  Hash_outputData(h, &e, &v, "2");
  printf("e=%f\tv=%f\n", e, sqrt(v));
  Hash_outputData(h, &e, &v, "$244d");
  printf("e=%f\tv=%f\n", e, sqrt(v));
  Hash_outputData_multiD(h, &e, &v, 2, "3", "3");
  Hash_outputData(h, &e, &v, "abcdefghi");
  Hash_outputData_multiD(h, &e, &v, 1, "33");
  printf("e=%f\tv=%f\n", e, sqrt(v));
  Hash_outputData(h, &e, &v, "fwefew");

  printf("\nDUMP TESTING RESULTS");
  printf("\nKEY                               MEAN      STD DEV\n");
  stringKeyList = (char **)malloc(size * sizeof(char *));
  vectorKeyList = (char ***)malloc(size * sizeof(char **));
  dimensionList = (int *)malloc(size * sizeof(int));
  a = (double **)malloc(size * sizeof(double *));
  for(i=0; i<size; i++)
    a[i] = (double *)malloc(2 * sizeof(double));

  Hash_dump(h, a, stringKeyList);

  for(i=0; i< size; i++)
    printf("%-25s%13.6f%13.6f\n", stringKeyList[i], a[i][0], sqrt(a[i][1]));

  printf("\nMULTID DUMP TESTING RESULTS");
  printf("\nKEY              DIMENSION         MEAN      STD DEV\n");
  Hash_dump_multiD(h, a, stringKeyList, dimensionList, vectorKeyList);
  
  for(i=0; i< size; i++)
  {
    printf("%-25s%d%13.6f%13.6f\n", stringKeyList[i], dimensionList[i],
        a[i][0], sqrt(a[i][1]));
    printf("           key vector:     ");
    for(j=0; j<dimensionList[i]; j++)
      printf("%-5s  ", vectorKeyList[i][j]);
    printf("\n");
  }
  
  Hash_del(h);
  
  free(stringKeyList);
  free(vectorKeyList);
  free(dimensionList);
  for(i=0; i<size; i++)
    free(a[i]);
  free(a);
}
