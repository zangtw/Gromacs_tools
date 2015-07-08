#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "hashStat.h"

int main()
{
  int size;
  double **a;
  char **stringKeyList;
  char **formatList;
  void **vectorKeyList;
  int *dimensionList;
  double e, v;
  int i, j;

  hashTable *h = Hash_init(7);
  
  Hash_insertKey(h, "s", "2");
  Hash_insertKey(h, "d", 2);
  Hash_insertKey(h, "ds", 200, "$256");
  Hash_insertKey(h, "sss", "abc", "def", "ghi");
  Hash_insertKey(h, "df", 4, 11.4);
  Hash_insertKey(h, "sssds", "@", "*(^B", "#(#D", 90, "F R3e");
  Hash_insertKey(h, "s", "$244d");

  Hash_removeKey(h, "s", "2");
  Hash_removeKey(h, "f", 2);
  Hash_removeKey(h, "s", "abc");
  Hash_removeKey(h, "ss", "abc", "def");
  Hash_removeKey(h, "s", "$244d");

  Hash_addData(h, 4, "s", "2");
  Hash_addData(h, 3.5, "s", "2");
  Hash_addData(h, 6.22, "f", 2.0);
  Hash_addData(h, 2, "ds", 200, "$256");
  Hash_addData(h, 5.5, "ss", "abc", "def");
  Hash_addData(h, 4.5, "s", "abcdef");
  Hash_addData(h, 7, "s", "$244d");
  Hash_addData(h, 6.7, "s", "$244d");
  Hash_addData(h, 6.7, "ss", "$244d", "zzz");
  Hash_addData(h, 398, "s", "zzz_fwc");
  Hash_addData(h, 104.355, "dd", 33, 214);
  Hash_addData(h, 4.56, "dd", 33, 214);

  Hash_getSize(h, &size);
  printf("\nsize=%d\n",size);

  printf("\nOUTPUT TESTING RESULTS\n");
  Hash_printData(h, &e, &v, "s", "2");
  printf("e=%f\tv=%f\n", e, sqrt(v));
  Hash_printData(h, &e, &v, "s", "$244d");
  printf("e=%f\tv=%f\n", e, sqrt(v));
  Hash_printData(h, &e, &v, "ds", 33, "214");
  Hash_printData(h, &e, &v, "abcdefghi");
  Hash_printData(h, &e, &v, "dd", 33, 214);
  printf("e=%f\tv=%f\n", e, sqrt(v));
  Hash_printData(h, &e, &v, "s", "fwefew");

  printf("\nSIMPLE DUMP TESTING RESULTS");
  printf("\nKEY                               MEAN      STD DEV\n");
  stringKeyList = (char **)malloc(size * sizeof(char *));
  vectorKeyList = (void **)malloc(size * sizeof(char *));
  dimensionList = (int *)malloc(size * sizeof(int));
  formatList = (char **)malloc(size * sizeof(char *));
  a = (double **)malloc(size * sizeof(double *));
  for(i=0; i<size; i++)
    a[i] = (double *)malloc(2 * sizeof(double));

  Hash_dump(h, a, stringKeyList, NULL, NULL, NULL);

  for(i=0; i< size; i++)
    printf("%-25s%13.6f%13.6f\n", stringKeyList[i], a[i][0], sqrt(a[i][1]));

  printf("\nCOMPLETE DUMP TESTING RESULTS");
  printf("\nKEY              DIMENSION             MEAN      STD DEV\tFORMAT\n");
  Hash_dump(h, a, stringKeyList, dimensionList, formatList, vectorKeyList);
  
  double *temp;
  for(i=0; i< size; i++)
  {
    printf("%-25s%-5d%13.6f%13.6f\t%s\n", stringKeyList[i], dimensionList[i], 
        a[i][0], sqrt(a[i][1]), formatList[i]);
    printf("           key vector:     ");
    for(j=0; j<dimensionList[i]; j++)
    {
      temp = (double *)vectorKeyList[i];
      
      if (formatList[i][j] == 's')
        printf("%-5s  ", *(char **)(temp + j));
      else if (formatList[i][j] == 'f')
        printf("%-5f  ", *(temp + j));
      else printf("%-5d  ", *(int *)(temp + j));
    }
    printf("\n");
  }
  
  Hash_del(h);
  
  free(stringKeyList);
  free(vectorKeyList);
  free(formatList);
  free(dimensionList);
  for(i=0; i<size; i++)
    free(a[i]);
  free(a);

  return 0;
}
