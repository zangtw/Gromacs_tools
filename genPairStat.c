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
#include "do_fit.h"
#include "matio.h"
#include "tpxio.h"
#include "viewit.h"
#include "gmx_ana.h"
#include "hashStat.h"

int main(int argc, char *argv[]) 
{
  const char *desc[] = { "Do statistics on pair distance", 
    "from an existing molecular dynamics trajectory." };
  gmx_bool bGREC = FALSE;
  gmx_bool bTraj = FALSE;
  int i, j;
  rvec           *x = NULL, *x_ref = NULL;
  matrix          box;
  char            buf[256];
  t_topology      top;
  output_env_t    oenv;
  int             ePBC;
  t_trxstatus    *status;
  int             natoms_trx;
  real            t;
  
  t_filenm        fnm[] =
  {
    { efTPS, NULL, NULL, ffREAD },
    { efLOG, "-n", NULL, ffREAD },
    { efTRX, "-f", NULL, ffOPTRD },
    { efLOG, "-o", NULL, ffWRITE },
  };
  t_pargs         pa[] = 
  {
    { "-grec", FALSE, etBOOL, {&bGREC}, 
      "Using GREC"}
  };
  #define NFILE asize(fnm)
  #define NPA asize(pa)
  
  CopyRight(stderr, argv[0]);
  parse_common_args(&argc, argv, PCA_CAN_TIME | PCA_TIME_UNIT | PCA_CAN_VIEW  
                    | PCA_BE_NICE, NFILE, fnm, NPA, pa, asize(desc),          
                    desc, 0, NULL, &oenv);
  
  read_tps_conf(ftp2fn(efTPS, NFILE, fnm), buf, &top, &ePBC, &x_ref,      
                       NULL, box, TRUE);
  
  bTraj = opt2bSet("-f", NFILE, fnm);
  
  /* obtain GREC_FA */
  int *GREC_ind;
  if (bGREC)
  {
    FILE *fgrec;
    fgrec = fopen("GREC", "r");
    int grec_size;

    fscanf(fgrec, "%d", &grec_size);
    snew(GREC_ind, grec_size);

    for(i=0; i<grec_size; i++)
      fscanf(fgrec, "%d", GREC_ind + i);

    fclose(fgrec);
  }
#define myIndex(i) (bGREC ? GREC_ind[i] : i)
  
  /* read index file */
  FILE *findex;
  findex = fopen(opt2fn("-n", NFILE, fnm), "r");
  int ind1, ind2;
  hashTable *h = Hash_init(113);

  while (fscanf(findex, "%d", &ind1) != EOF)
  {
    fscanf(findex, "%d", &ind2);
    Hash_insertKey(h, "dd", ind1, ind2);
  }
  fclose(findex);

  int size;
  void ***vectorKeyList;
  
  Hash_getSize(h, &size);
  snew(vectorKeyList, size);
  
  Hash_dump(h, NULL, NULL, NULL, NULL, vectorKeyList);

  printf("size=%d\n", size);
  
  /* set reference value from the topology file */
  double diff;
  for(i=0; i< size; i++)
  {
    ind1 = *(int *)(vectorKeyList[i][0]);
    ind2 = *(int *)(vectorKeyList[i][1]);
  
    /* indicating coordinate data is missing in the topology file */
    if(fabs(x_ref[myIndex(ind1)][0]) < 5e-6 && 
       fabs(x_ref[myIndex(ind1)][1]) < 5e-6 &&
       fabs(x_ref[myIndex(ind1)][2]) < 5e-6 )
          continue;
    if(fabs(x_ref[myIndex(ind2)][0]) < 5e-6 && 
       fabs(x_ref[myIndex(ind2)][1]) < 5e-6 &&
       fabs(x_ref[myIndex(ind2)][2]) < 5e-6 )
          continue;

    diff = 0;
    diff += (x_ref[myIndex(ind1)][0] - x_ref[myIndex(ind2)][0]) * 
            (x_ref[myIndex(ind1)][0] - x_ref[myIndex(ind2)][0]);
    diff += (x_ref[myIndex(ind1)][1] - x_ref[myIndex(ind2)][1]) * 
            (x_ref[myIndex(ind1)][1] - x_ref[myIndex(ind2)][1]);
    diff += (x_ref[myIndex(ind1)][2] - x_ref[myIndex(ind2)][2]) * 
            (x_ref[myIndex(ind1)][2] - x_ref[myIndex(ind2)][2]);
    diff = sqrt(diff);

    Hash_setReferenceValue(h, diff, "dd", ind1, ind2);
  }

  if(bTraj)
  {
    /* read first frame */
    natoms_trx = read_first_x(oenv, &status, opt2fn("-f", NFILE, fnm), &t, 
                            &x, box);
    if (natoms_trx != top.atoms.nr)
      fprintf(stderr, 
              "\nWARNING: topology has %d atoms, whereas trajectory has %d\n",
              top.atoms.nr, natoms_trx);
  
    /* start looping over frames: */
    do
    {
      for(i=0; i< size; i++)
      {
        ind1 = *(int *)(vectorKeyList[i][0]);
        ind2 = *(int *)(vectorKeyList[i][1]);
    
        diff = 0;
        diff += (x[myIndex(ind1)][0] - x[myIndex(ind2)][0]) * 
                (x[myIndex(ind1)][0] - x[myIndex(ind2)][0]);
        diff += (x[myIndex(ind1)][1] - x[myIndex(ind2)][1]) * 
                (x[myIndex(ind1)][1] - x[myIndex(ind2)][1]);
        diff += (x[myIndex(ind1)][2] - x[myIndex(ind2)][2]) * 
                (x[myIndex(ind1)][2] - x[myIndex(ind2)][2]);
        diff = sqrt(diff);

        Hash_addData(h, diff, "dd", ind1, ind2);
      }
    }
    while (read_next_x(oenv, status, &t, natoms_trx, x, box));

    close_trj(status);
  }

  /* output the statistics */
  FILE *fopt;
  fopt = fopen(opt2fn("-o", NFILE, fnm), "w");
  double **a;
  snew(a, size);
  for(i=0; i<size; i++)
    snew(a[i], 3);
  
  Hash_dump(h, a, NULL, NULL, NULL, NULL);
  
  for(i=0; i<size; i++)
  {
    ind1 = *(int *)(vectorKeyList[i][0]);
    ind2 = *(int *)(vectorKeyList[i][1]);
    /* using standard deviation */

    fprintf(fopt, "%4d    %4d    %13.6f    %13.6f    %13.6f\n", 
        ind1, ind2, a[i][0], sqrt(a[i][1]), a[i][2]);
  }

  fclose(fopt);

  Hash_del(h);

  sfree(x_ref);
  sfree(x);

  sfree(GREC_ind);
  sfree(vectorKeyList);

  for(i=0; i<size; i++)
    sfree(a[i]);
  sfree(a);
}
