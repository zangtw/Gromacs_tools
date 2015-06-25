#define GMX_DOUBLE
#define GMX_INTEGER_BIG_ENDIAN 
#define GMXLIBDIR "/home/tz4/softwares/share/gromacs/top"
#define PACKAGE  "Gromacs"
#define VERSION  "4.6.3"
#define USE_VERSION_H

#include "smalloc.h"
#include <math.h>
#include "macros.h"
#include "typedefs.h"
#include "xvgr.h"
#include "copyrite.h"
#include "statutil.h"
#include "string2.h"
#include "vec.h"
#include "index.h"
#include "gmx_fatal.h"
#include "futil.h"
#include "princ.h"
#include "rmpbc.h"
#include "matio.h"
#include "tpxio.h"
#include "cmat.h"
#include "viewit.h"
#include "gmx_ana.h"
#include "genContactMap.h"

#define myMap (*p_myMap)

void contactMap_init(contactMap *p_myMap, int Nr, int Nc)
{
  myMap.Nr = Nr;
  myMap.Nc = Nc;
  myMap.count = 0;

  int i;

  myMap.map = (real **)malloc(sizeof(real *) * Nr); 
  for(i=0; i<Nr; i++)
    myMap.map[i] = (real *)malloc(sizeof(real) * Nc);

  /* init */
  contactMap_reset(p_myMap);
}

void contactMap_del(contactMap *p_myMap)
{
  int i;

  for(i=0; i< myMap.Nr; i++)
    free(myMap.map[i]);
  free(myMap.map); 
}

void contactMap_reset(contactMap *p_myMap)
{
  int i, j;
  
  for(i=0; i< myMap.Nr; i++)
    for(j=0; j< myMap.Nc; j++)
      myMap.map[i][j] = 0;

  myMap.count = 0;
}

void contactMap_genMap(contactMap *p_myMap, real *x, int *group_idx1,
		                   int *group_idx2, real cutoff2)
{
  int i, j, ind1_3, ind2_3;
  real diff, dist2;

  myMap.count = 0;

  for(i=0; i< myMap.Nr; i++)
	{
		ind1_3 = group_idx1[i] + group_idx1[i] + group_idx1[i];

    for(j=i; j< myMap.Nc; j++)
    {
			ind2_3 = group_idx2[j] + group_idx2[j] + group_idx2[j];
    
			diff = x[ind1_3] - x[ind2_3];
      dist2 = diff * diff;

			diff = x[ind1_3 + 1] - x[ind2_3 + 1];
      dist2 += diff * diff;
      
			diff = x[ind1_3 + 2] - x[ind2_3 + 2];
      dist2 += diff * diff;

      if(dist2 <= cutoff2)
        myMap.count++;
      myMap.map[i][j] = sqrt(dist2); //need to find a quick sqrt algorithm..
    }
	}
}

void contactMap_calcMapDist(contactMap *p_myMap, contactMap *p_refMap, int *d)
{
  int i ,j;

  if(myMap.Nr != (*p_refMap).Nr)
  {
    printf("Nr inconsistent!\n");
    exit(0);
  }
  if(myMap.Nc != (*p_refMap).Nc)
  {
    printf("Nc inconsistent!\n");
    exit(0);
  }
  
	for(i=0, *d=0; i< myMap.Nr; i++)
    for(j=0; j< myMap.Nc; j++)
      if(myMap.map[i][j] != (*p_refMap).map[i][j])
        (*d)++;
}  

/**************** Followings are related to mapStat **************************/

typedef struct mapStat_t mapStat;
struct mapStat_t{
	real s;
	real se;
	real se2;
};

static void contactMap_mapStat_kernel_d(mapStat ***myStat, int Nr, int Nc)
{
	mapStat **stat = *myStat;
	int i, j;
	
	if(stat != NULL)
	{
		for(i=0; i<Nr; i++)
			if(stat[i] != NULL) free(stat[i]);
		
		free(stat);
	}
	stat = NULL;
}

static void contactMap_mapStat_kernel_a(mapStat ***myStat, int *Nr, int *Nc, 
		                             contactMap *p_myMap)
{
	mapStat **stat;
	int i, j;
	real s_old, se_old;
	
	/* init */ 
	if(*Nr==0) *Nr = myMap.Nr;
	if(*Nc==0) *Nc = myMap.Nc;
	if(*myStat == NULL)
	{
		stat = (mapStat **)malloc(sizeof(mapStat *) * myMap.Nr); 
		for(i=0; i<myMap.Nr; i++)
		{
			stat[i] = (mapStat *)malloc(sizeof(mapStat) * myMap.Nc);
			
			for(j=0; j<myMap.Nc; j++) 
			{
				stat[i][j].s = 0;
				stat[i][j].se = 0;
				stat[i][j].se2 = 0;
		}
	
		*myStat = stat;
	}
	
	/* add new map data to stat */
	stat = *myStat;

	if(myMap.Nr != (*Nr) || myMap.Nc != (*Nc))
	{
		printf("ERROR: the size of the input(map) is different from the current!");
		exit(1);
	}

	for(i=0; i<myMap.Nr; i++)
		for(j=0; j<myMap.Nc; j++)
		{
			s_old = stat[i][j].s;
			se_old = stat[i][j].se;

			stat[i][j].s += 1;
			stat[i][j].se += myMap.map[i][j];
			stat[i][j].se2 += (myMap.map[i][j] - stat[i][j].se / stat[i][j].s) 
				                * (myMap.map[i][j] - se_old / s_old);
		}

}
		
#define round(a) (int)(a+0.5)
static void contactMap_mapStat_kernel_p(mapStat **stat, int Nr, int Nc, FILE *fp)
{
	static char title[] = "static char * contactMap[] = \n{\n";
	static char end[] = "};";
	static const char colormap[] = "ABCDEFGHIJK";
	int i, j;
	real r, g, b;
	real res;

	fprintf(fp, "%s", title);
	fprintf(fp, "\"%d %d 11 1\",\n", Nr, Nc);
	/* print rgb code */
	for(i=0; i<=10; i++)
	{
		r = 0.1 * i;
		g = 0.1 * i;
		b = 0.1 * i;
    fprintf(fp, "\"%c c #%02X%02X%02X \",\n", colormap[i], 
				        (unsigned int)round(255*r), (unsigned int)round(255*g),
								(unsigned int)round(255*b));
	}

	for(i=0; i<Nr; i++)
	{
		fprintf(fp, "\"");
		
		for(j=0; j<Nc; j++)
		{
			if(stat[i][j].s == 0) res = 0;
			else res = stat[i][j].se / stat[i][j].s;

			fprintf(fp, "%c", colormap[(int)(res * 10)]);
		}
		
		fprintf(fp, "\",\n");
	}
	
	fprintf(fp, "%s", end);
}
		
static void contactMap_mapStat_kernel_mv(mapStat **stat, int Nr, int Nc, 
		                                    contactMap *p_myMap, int flag)
{
	int i, j;
	real res;

	for(i=0; i<Nr; i++)
		for(j=0; j<Nc; j++)
		{
			if(stat[i][j].s == 0) res = 0;
			else if(flag == 1) res = stat[i][j].se / stat[i][j].s;
			else res = sqrt( stat[i][j].se2 / stat[i][j].s );

			myMap.map[i][j] = res;
			if (flag == 1 && res <= 0.2) myMap.count ++;
		}
}

static void contactMap_mapStat_kernel(char cmd, void *p)
{
	static int Nr = 0;
	static int Nc = 0;
	static mapStat **stat = NULL;
	int i, j;
	
	switch(cmd)
	{
	case 'd':
		contactMap_mapStat_kernel_d(&stat, Nr, Nc);
		break;
	case 'a':
		contactMap_mapStat_kernel_a(&stat, &Nr, &Nc, (contactMap *)p);
		break;
	case 'p':
		contactMap_mapStat_kernel_p(stat, Nr, Nc, (FILE *)p);
		break;
	case 'm':
		contactMap_mapStat_kernel_mv(stat, Nr, Nc, (contactMap *)p, 1);
		break;
	case 'v':
		contactMap_mapStat_kernel_mv(stat, Nr, Nc, (contactMap *)p, 2);
		break;
	default:
		break;
	}
}

void contactMap_mapStat_addMap(contactMap *p_myMap)
{
	contactMap_mapStat_kernel('a', (void *)p_myMap);
}

void contactMap_mapStat_del()
{
	contactMap_mapStat_kernel('d', NULL);
}

void contactMap_mapStat_print(FILE *fp)
{
	contactMap_mapStat_kernel('p', (void *)fp);
}

void contactMap_mapStat_getMean(contactMap *p_meanMap)
{
	contactMap_mapStat_kernel('m', (void *)p_meanMap);
}

void contactMap_mapStat_getVar(contactMap *p_varMap)
{
	contactMap_mapStat_kernel('v', (void *)p_varMap);
}

#undef myMap
/*------------------------ End of contactMap methods --------------------*/

#include <stdio.h>

int main(int argc, char *argv[])
{
  const char  *desc[] =
  {
    "Generate contact maps of a target trajectory.",
    "Option [TT]-cutoff[tt] is the cutoff distance (in nm).",
  };
	t_filenm        fnm[] =
  {
    { efTPS, NULL,    NULL, ffREAD },
    { efTRX, "-f",    NULL, ffREAD },
    { efLOG, "-o",    NULL, ffWRITE },
    { efLOG, "-stat", "dm", ffOPTWR },
  };
#define NFILE asize(fnm)

	real             cutoff = 0.4, cutoff2;
	t_pargs          pa[] =
	{
		{ "-cutoff", FALSE, etREAL,
			{&cutoff}, "Cutoff length for generating contact maps"}
	};
  
  output_env_t    oenv;
	gmx_bool        bTop;
  char            buf[256];
  t_topology      top;
  int             ePBC;
	rvec           *x = NULL, *xp = NULL; 
  matrix          box;
	int             imap1, imap2;
	atom_id        *ind_map1 = NULL, *ind_map2 = NULL;
	char           *gn_map1 = NULL, *gn_map2 = NULL;
  FILE           *fout = NULL, *fstat = NULL;
  contactMap      map, refmap, meanMap, varMap;
  t_trxstatus    *status;
	real            t;
	int             natoms_trx;
	int             d;
	gmx_bool        bStat;
  
	CopyRight(stderr, argv[0]);
  parse_common_args(&argc, argv, PCA_CAN_TIME | PCA_TIME_UNIT | PCA_CAN_VIEW  
                    | PCA_BE_NICE, NFILE, fnm, asize(pa), pa, asize(desc),    
										desc, 0, NULL, &oenv);	
  
	bTop = read_tps_conf(ftp2fn(efTPS, NFILE, fnm), buf, &top, &ePBC, &xp,      
                       NULL, box, TRUE);
	cutoff2 = cutoff * cutoff;
  bStat = opt2bSet("-stat", NFILE, fnm);
  
	fprintf(stderr, "Select two groups for generating contact maps\n");
  get_index(&(top.atoms), ftp2fn_null(efNDX, NFILE, fnm), 1, &imap1, &ind_map1, 
  		      &gn_map1);
  get_index(&(top.atoms), ftp2fn_null(efNDX, NFILE, fnm), 1, &imap2, &ind_map2, 
  		      &gn_map2);
	contactMap_init(&refmap, imap1, imap2);
  
	/* calc the contact map of the reference structure */
	contactMap_genMap(&refmap, (real *)xp, ind_map1, ind_map2, cutoff2);
	
  /* read first frame */
  natoms_trx = read_first_x(oenv, &status, opt2fn("-f", NFILE, fnm), &t, 
			                      &x, box);
  if (natoms_trx != top.atoms.nr)
  {
		fprintf(stderr, 
				    "\nWARNING: topology has %d atoms, whereas trajectory has %d\n",
						top.atoms.nr, natoms_trx);
  }
      
	contactMap_init(&map, imap1, imap2);
    
	fout = ffopen(opt2fn("-o", NFILE, fnm), "w");
	
	/* start looping over frames: */
  do
  {
    /* calc the contact map of the current structure */
    contactMap_genMap(&map, (real *)x, ind_map1, ind_map2, cutoff2);
    
    fprintf(fout, "%13.6f %d\t\t", t, map.count);

		if(bStat)
			contactMap_mapStat_addMap(&map);
    
    contactMap_calcMapDist(&map, &refmap, &d);
    fprintf(fout, "%d\n", d);
  }
  while (read_next_x(oenv, status, &t, natoms_trx, x, box));
  close_trj(status);
		
	if(bStat)
	{
		fstat = ffopen(opt2fn("-stat", NFILE, fnm), "w");
		contactMap_mapStat_print(fstat);
	
		contactMap_init(&meanMap, imap1, imap2);
		contactMap_mapStat_getMean(&meanMap);
		
		contactMap_init(&varMap, imap1, imap2);
		contactMap_mapStat_getVar(&varMap);

		contactMap_mapStat_del();
		ffclose(fstat);
	}

  contactMap_calcMapDist(&meanMap, &refmap, &d);
  fprintf(fout, "    -1.000000 %d\t\t", meanMap.count);
  fprintf(fout, "%d\n", d);
	ffclose(fout);

	contactMap_del(&map);
	contactMap_del(&refmap);
	contactMap_del(&meanMap);
	contactMap_del(&varMap);

  return 0;
}
