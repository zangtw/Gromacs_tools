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
  void ***vectorKeyList;
  int *dimensionList;
  double e, v, ref;
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

  Hash_setReferenceValue(h, 8.8, "s", "2");
  Hash_setReferenceValue(h, 2.5, "s", "2");
  Hash_setReferenceValue(h, 5.6, "f", 2.0);
  Hash_setReferenceValue(h, 5.33, "ssf", "asc", "efe", 4.66);

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
  Hash_printData(h, &e, &v, &ref, "s", "2");
  printf("e=%f\tv=%f\tref=%f\n", e, sqrt(v), ref);
  Hash_printData(h, &e, &v, &ref, "s", "$244d");
  printf("e=%f\tv=%f\tref=%f\n", e, sqrt(v), ref);
  Hash_printData(h, &e, &v, &ref, "ds", 33, "214");
  Hash_printData(h, &e, &v, NULL, "abcdefghi");
  Hash_printData(h, &e, &v, NULL, "dd", 33, 214);
  printf("e=%f\tv=%f\tref=%f\n", e, sqrt(v), ref);
  Hash_printData(h, &e, &v, NULL, "s", "fwefew");

  printf("\nSIMPLE DUMP TESTING RESULTS");
  printf("\nKEY                               MEAN      STD DEV          REF\n");
  stringKeyList = (char **)malloc(size * sizeof(char *));
  vectorKeyList = (void ***)malloc(size * sizeof(void **));
  dimensionList = (int *)malloc(size * sizeof(int));
  formatList = (char **)malloc(size * sizeof(char *));
  a = (double **)malloc(size * sizeof(double *));
  for(i=0; i<size; i++)
    a[i] = (double *)malloc(3 * sizeof(double));

  Hash_dump(h, a, stringKeyList, NULL, NULL, NULL);

  for(i=0; i< size; i++)
    printf("%-25s%13.6f%13.6f%13.6f\n", stringKeyList[i], a[i][0], sqrt(a[i][1]), a[i][2]);

  printf("\nCOMPLETE DUMP TESTING RESULTS");
  printf("\nKEY              DIMENSION             MEAN      STD DEV          REF\tFORMAT\n");
  Hash_dump(h, a, stringKeyList, dimensionList, formatList, vectorKeyList);
  
  for(i=0; i< size; i++)
  {
    printf("%-25s%-5d%13.6f%13.6f%13.6f\t%s\n", stringKeyList[i], dimensionList[i], 
        a[i][0], sqrt(a[i][1]), a[i][2], formatList[i]);
    printf("           key vector:     ");
    for(j=0; j<dimensionList[i]; j++)
    {
      if (formatList[i][j] == 's')
        printf("%-5s  ", (char *)(vectorKeyList[i][j]));
      else if (formatList[i][j] == 'f')
        printf("%-5f  ", *(double *)(vectorKeyList[i][j]));
      else printf("%-5d  ", *(int *)(vectorKeyList[i][j]));
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
