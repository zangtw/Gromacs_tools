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
#include "cmat.h"
#include "viewit.h"
#include "gmx_ana.h"

static int obtainCoorFromPDB(FILE *fp, rvec **x)
{
	int natoms;
  char title[STRLEN];
	int ePBC, model_nr;
	t_atoms atoms;
  matrix boxpdb;
    
	get_pdb_coordnum(fp, &natoms);
  if (natoms == 0)
		gmx_fatal(FARGS, "\nNo coordinates in pdb file\n");
  
	frewind(fp);
	srenew(*x, natoms);

	atoms.nr      = natoms;
  atoms.atom    = NULL;
  atoms.pdbinfo = NULL;
	read_pdbfile(fp, title, &model_nr, &atoms, *x, &ePBC, boxpdb, TRUE, NULL);

	return natoms;
}

int main(int argc, char *argv[])
{
  const char *desc[] = { "Generate structure in pdb format",                  \
		"after least-square fit to a reference structure." };
  static gmx_bool bMassWeighted = TRUE;
  int             natoms_trx, natoms;
  int             i;
  real            t, *w_rls, *w_rms;
  gmx_bool        bMass;
  t_topology      top;
  int             ePBC;
  
  matrix          box;
  rvec            *x = NULL, *xp = NULL, *xAv = NULL;
	rvec            *xAv2 = NULL, *xFluc = NULL;
  char             buf[256];
  FILE            *flist, *fp, *fout, *ffluc;
	int             nfiles;
  char            line[STRLEN];
  real            rls;
  gmx_bool			  bTop = FALSE, bFluc = FALSE;
	gmx_bool				bSelf = TRUE, bStd = TRUE, bR = FALSE;
  int             ifit, irms;
  atom_id         *ind_fit, *ind_rms;
  char            *gn_fit, *gn_rms;
  output_env_t    oenv;
  
  t_filenm        fnm[] =
  {
    { efTPS, NULL, NULL, ffREAD },
    { efLOG, "-f", NULL, ffREAD },
    { efPDB, "-o", NULL, ffWRITE },
    { efLOG, "-fluc", NULL, ffOPTWR }
  };
	t_pargs         pa[] = 
	{
		{ "-self", FALSE, etBOOL, {&bSelf}, 
			"Calculate self-standard deviation (fluctuation) if TRUE (default); calculate standard deviation (displacement) with respect to the referenece structure if FALSE. "},
		{ "-std", FALSE, etBOOL, {&bStd}, 
			"Calculate standard deviation (root of variance) if TRUE (default); calculate variance instead if FALSE. "},
		{ "-r", FALSE, etBOOL, {&bR}, 
			"Calculate radial deviation if TRUE; calculate deviation in (x,y,z) if FALSE (default). "}
	};
#define NFILE asize(fnm)
#define NPA asize(pa)
  
  CopyRight(stderr, argv[0]);
  parse_common_args(&argc, argv, PCA_CAN_TIME | PCA_TIME_UNIT | PCA_CAN_VIEW  
                    | PCA_BE_NICE, NFILE, fnm, NPA, pa, asize(desc),          
  									desc, 0, NULL, &oenv);
  
  bTop = read_tps_conf(ftp2fn(efTPS, NFILE, fnm), buf, &top, &ePBC, &xp,      
                       NULL, box, TRUE);
  snew(w_rls, top.atoms.nr);
  snew(w_rms, top.atoms.nr);
	bFluc = opt2bSet("-fluc", NFILE, fnm);
  
  fprintf(stderr, "Select group for least squares fit\n");
  get_index(&(top.atoms), ftp2fn_null(efNDX, NFILE, fnm), 1, &ifit, &ind_fit, 
  		      &gn_fit);
  
  if (ifit < 3)
  	gmx_fatal(FARGS, "Need >= 3 points to fit!\n" );
  
  bMass = FALSE;
  for (i = 0; i < ifit; i++)
  {
    if (bMassWeighted)
      w_rls[ind_fit[i]] = top.atoms.atom[ind_fit[i]].m;
    else
      w_rls[ind_fit[i]] = 1;
    
  	bMass = bMass || (top.atoms.atom[ind_fit[i]].m != 0);
  }
  if (!bMass)
  {
    fprintf(stderr, "All masses in the fit group are 0, using masses of 1\n");
    for (i = 0; i < ifit; i++)
			w_rls[ind_fit[i]] = 1;
  }
  
  get_index(&(top.atoms), ftp2fn_null(efNDX, NFILE, fnm),                     
            1, &irms, &ind_rms, &gn_rms);
  
  bMass = FALSE;
  for (i = 0; i < irms; i++)
  {
    if (bMassWeighted)
      w_rms[ind_rms[i]] = top.atoms.atom[ind_rms[i]].m;
    else
      w_rms[ind_rms[i]] = 1.0;
      
  	bMass = bMass || (top.atoms.atom[ind_rms[i]].m != 0);
  }
  if (!bMass)
  {
    fprintf(stderr, "All masses in the target group are 0, using masses of 1\n");
    for (i = 0; i < irms; i++)
      w_rms[ind_rms[i]] = 1;
  }
  
  /* Prepare reference structure */
  reset_x(ifit, ind_fit, top.atoms.nr, NULL, xp, w_rls);
  
	flist = fopen(opt2fn("-f", NFILE, fnm), "r");
	nfiles = 0;

	/* loop over target structures */
	while (fgets2(line, STRLEN, flist))
	{
		fp = fopen(line, "r");

		natoms_trx = obtainCoorFromPDB(fp, &x);

  	if (natoms_trx != top.atoms.nr)
  	{
  	  fprintf(stderr,                                                          
  	      "\nWARNING: topology has %d atoms, whereas trajectory has %d\n", 
  	      top.atoms.nr, natoms_trx);
  	}
  	natoms = min(top.atoms.nr, natoms_trx);
		
		if (xAv == NULL) snew(xAv, natoms);
		if (bFluc)
		{
			if (xAv2 == NULL) snew(xAv2, ifit);
			if (xFluc == NULL) snew(xFluc, ifit);
		}
  	
  	/* Prepare target structure */
  	reset_x(ifit, ind_fit, natoms, NULL, x, w_rls);
  	
  	/* do the least squares fit to original structure */
  	do_fit(natoms, w_rls, xp, x);

		for (i = 0; i< natoms; i++)
		{
			xAv[i][0] += x[i][0];
			xAv[i][1] += x[i][1];
			xAv[i][2] += x[i][2];
		}

		if (bFluc)
		{
			for (i = 0; i< ifit; i++)
			{
				xAv2[i][0] += x[ind_fit[i]][0] * x[ind_fit[i]][0];
				xAv2[i][1] += x[ind_fit[i]][1] * x[ind_fit[i]][1];
				xAv2[i][2] += x[ind_fit[i]][2] * x[ind_fit[i]][2];
			}
		}
  	
		/* calc RMSD */
  	rls = calc_similar_ind(FALSE, irms, ind_rms, w_rms, x, xp);

		nfiles++;
		fprintf(stderr, "\nnfiles: %d   RMSD: %12.7f\n", nfiles, rls);
		
		fclose(fp);
	}
		
	fout = fopen(opt2fn("-o", NFILE, fnm), "w");
	real temp = 10.0 / nfiles;
	for (i = 0; i< natoms; i++)
	{
		xAv[i][0] *= temp;
		xAv[i][1] *= temp;
		xAv[i][2] *= temp;

		fprintf(fout, "%8.3f%8.3f%8.3f\n", xAv[i][0], xAv[i][1], xAv[i][2]);
	}
	fclose(fout);

	if (bFluc)
	{
		real Rfluc = 0;

		ffluc = fopen(opt2fn("-fluc", NFILE, fnm), "w");
		temp = 100.0 / nfiles;
		for (i = 0; i< ifit; i++)
		{
			xAv2[i][0] *= temp;
			xAv2[i][1] *= temp;
			xAv2[i][2] *= temp;

			if (bSelf)
			{
				xFluc[i][0] = xAv2[i][0] - xAv[ind_fit[i]][0] * xAv[ind_fit[i]][0];
				xFluc[i][1] = xAv2[i][1] - xAv[ind_fit[i]][1] * xAv[ind_fit[i]][1];
				xFluc[i][2] = xAv2[i][2] - xAv[ind_fit[i]][2] * xAv[ind_fit[i]][2];
			}
			else
			{
				real mu = xp[ind_fit[i]][0] * 10;
				xFluc[i][0] = xAv2[i][0];
				xFluc[i][0] += (mu - xAv[ind_fit[i]][0] - xAv[ind_fit[i]][0]) * mu;
				
				mu = xp[ind_fit[i]][1] * 10;
				xFluc[i][1] = xAv2[i][1];
				xFluc[i][1] += (mu - xAv[ind_fit[i]][1] - xAv[ind_fit[i]][1]) * mu;
				
				mu = xp[ind_fit[i]][2] * 10;
				xFluc[i][2] = xAv2[i][2];
				xFluc[i][2] += (mu - xAv[ind_fit[i]][2] - xAv[ind_fit[i]][2]) * mu;
			}

			xFluc[i][0] = xFluc[i][0] > 0 ? xFluc[i][0] : (-xFluc[i][0]);
			xFluc[i][1] = xFluc[i][1] > 0 ? xFluc[i][1] : (-xFluc[i][1]);
			xFluc[i][2] = xFluc[i][2] > 0 ? xFluc[i][2] : (-xFluc[i][2]);

			if(bR)
			{
				Rfluc += xFluc[i][0];
				Rfluc += xFluc[i][1];
				Rfluc += xFluc[i][2];
			}
			
			if(bStd)
			{
				xFluc[i][0] = sqrt(xFluc[i][0]);
				xFluc[i][1] = sqrt(xFluc[i][1]);
				xFluc[i][2] = sqrt(xFluc[i][2]);

				if(bR)
					Rfluc = sqrt(Rfluc);
			}
			
			if(bR)
				fprintf(ffluc, "%f\n", Rfluc);
			else
			{
				fprintf(ffluc, "%f\n", xFluc[i][0]);
				fprintf(ffluc, "%f\n", xFluc[i][1]);
				fprintf(ffluc, "%f\n", xFluc[i][2]);
			}
		}
		fclose(ffluc);
	}
  
	sfree(w_rls);
  sfree(w_rms);
  sfree(ind_fit);
	sfree(ind_rms);
	sfree(x);
	sfree(xp);
	sfree(xAv);
	if (bFluc)
	{
		sfree(xAv2);
		sfree(xFluc);
	}
  
  return 0;
}
