#ifndef CONTACTMAP_H
#define CONTACTMAP_H

#ifndef bool
#define bool int
#endif

#ifndef HAVE_REAL
#define real float
#endif

typedef struct contactMap_t contactMap;
struct contactMap_t{
  int Nr, Nc;
  int count;  /* tot number of contacts */
  real **map;  /* (binary) contact map with Nr rows and Nc columns */
};

void contactMap_init(contactMap *p_myMap, int Nr, int Nc);
void contactMap_del(contactMap *p_myMap);
/* manually reset the map. Map is also reseted in genMap(automatically). */
void contactMap_reset(contactMap *p_myMap);
void contactMap_genMap(contactMap *p_myMap, real *x, int *group_idx1,
		                   int *group_idx2, real cutoff2);
void contactMap_calcMapDist(contactMap *p_myMap, contactMap *p_refMap, int *d);
void contactMap_mapStat_addMap(contactMap *p_myMap);
void contactMap_mapStat_del();
void contactMap_mapStat_print(FILE *fp);
void contactMap_mapStat_getMean(contactMap *p_meanMap);
void contactMap_mapStat_getVar(contactMap *p_varMap);

#endif
