/*************************************************************************/
/*                                                                       */
/*  Copyright (c) 1994 Stanford University                               */
/*                                                                       */
/*  All rights reserved.                                                 */
/*                                                                       */
/*  Permission is given to use, copy, and modify this software for any   */
/*  non-commercial purpose as long as this copyright notice is not       */
/*  removed.  All other uses, including redistribution in whole or in    */
/*  part, are forbidden without prior written permission.                */
/*                                                                       */
/*  This software is provided with absolutely no warranty and no         */
/*  support.                                                             */
/*                                                                       */
/*************************************************************************/

/*  Usage:   water < infile,
    where infile has 10 fields which can be described in order as 
    follows:
    
    TSTEP:   the physical time interval (in sec) between timesteps.  
    Good default is 1e-15.
    NMOL:    the number of molecules to be simulated.
    NSTEP:   the number of timesteps to be simulated.
    NORDER:  the order of the predictor-corrector method to be used.
    set this to 6.
    NSAVE:   the frequency with which to save data in data collection.
    Set to 0 always.
    NRST:    the frequency with which to write RST file: set to 0 always (not used). 
    NPRINT:  the frequency with which to compute potential energy. 
    i.e. the routine POTENG is called every NPRINT timesteps.
    It also computes intermolecular as well as intramolecular
    interactions, and hence is very expensive.
    NFMC:    Not used (historical artifact).  Set to anything, say 0.
    NumProcs: the number of processors to be used.
    CUTOFF:  the cutoff radius to be used (in Angstrom, 
    floating-point).  In a real simulation, this
    will be set to 0 here in which case the program will 
    compute it itself (and set it to about 11 Angstrom.   
    It can be set by the user if they want
    to use an artificially small cutoff radius, for example
    to control the number of boxes created for small problems 
    (and not have fewer boxes than processors).  
    */

MAIN_ENV
#include "stdio.h"
#include "split.h"

/*  include files for declarations  */
#define extern
#include "parameters.h"
#include "mdvar.h"
#include "water.h"
#include "wwpot.h"
#include "cnst.h"
#include "mddata.h"
#include "fileio.h"
#include "frcnst.h"
#include "global.h"
/* #undef extern */

struct GlobalMemory *gl;        /* pointer to the Global Memory
                                   structure, which contains the lock,
                                   barrier, and some scalar variables */


int NSTEP, NSAVE, NRST, NPRINT,NFMC;
int NORD1;
int II;                         /*  variables explained in common.h */
int i;
int NDATA;
int   NFRST=11;
int  NFSV=10;
int  LKT=0;

int StartMol[MAXPROCS+1];       /* number of the first molecule
                                   to be handled by this process; used
                                   for static scheduling     */ 
int MolsPerProc;                /* number of mols per processor */ 
int NumProcs;                   /* number of processors being used; 
                                   run-time input           */

void WorkStart();

main(argc, argv)
  char **argv;
{
    int mol, func, dir, atom;
    double XTT, MDMAIN();
    unsigned ProcID = 0;
    
    /* default values for the control parameters of the driver */
    /* are in parameters.h */

    if ((argc == 2) &&((strncmp(argv[1],"-h",strlen("-h")) == 0) || (strncmp(argv[1],"-H",strlen("-H")) == 0))) {
        printf("Usage:  WATER-NSQUARED < infile, where the contents of infile can be\nobtained from the comments at the top of water.C and the first scanf \nin main() in water.C\n\n");
        exit(0);
    }
    
    /*  POSSIBLE ENHANCEMENT:  Here's where one might bind the main process 
        (process 0) to a processor if one wanted to. Others can be bound in 
        the WorkStart routine.
        */
    
    six = stdout;   /* output file */
    
    TEMP  =298.0;
    RHO   =0.9980;
    CUTOFF=0.0;
    
    /* read input */
    
    if (scanf("%lf%d%d%d%d%d%d%d%d%lf",&TSTEP, &NMOL, &NSTEP, &NORDER, 
              &NSAVE, &NRST, &NPRINT, &NFMC,&NumProcs, &CUTOFF) != 10)
        fprintf(stderr,"ERROR: Usage: water < infile, which must have 10 fields, see SPLASH documentation or comment at top of water.C\n");
    
    if (NMOL > MAXLCKS) {
        fprintf(stderr, "Just so you know ... Lock array in global.H has size %d < %d (NMOL)\n code will still run correctly but there may be lock contention\n\n", MAXLCKS, NMOL);
    }
    
    printf("Using %d procs on %d steps of %d mols\n", NumProcs, NSTEP, NMOL);
    printf("Other parameters:\n\tTSTEP = %8.2e\n\tNORDER = %d\n\tNSAVE = %d\n",TSTEP,NORDER,NSAVE);
    printf("\tNRST = %d\n\tNPRINT = %d\n\tNFMC = %d\n\tCUTOFF = %lf\n\n",NRST,NPRINT,NFMC,CUTOFF);
    
    
    /* SET UP SCALING FACTORS AND CONSTANTS */
    
    NORD1=NORDER+1;
    
    CNSTNT(NORD1,TLC);  /* sub. call to set up constants */
    
    
    { /* Do memory initializations */
        int pid;
        unsigned mol_size = sizeof(molecule_type) * NMOL;
        unsigned gmem_size = sizeof(struct GlobalMemory);
        
        /*  POSSIBLE ENHANCEMENT:  One might bind the first process to 
            a processor here, even before the other (child) processes are 
            bound later in mdmain(). 
            */
        
        MAIN_INITENV(,70000000,);  /* macro call to initialize
                                      shared memory etc. */
        
        /* allocate space for main (VAR) data structure as well as
           synchronization variables */
        
        /*  POSSIBLE ENHANCEMENT: One might want to allocate a process's
            portion of the VAR array and what it points to in its local 
            memory */
        
        VAR = (molecule_type *) G_MALLOC(mol_size);
        gl = (struct GlobalMemory *) G_MALLOC(gmem_size);
        
        /*  POSSIBLE ENHANCEMENT: One might want to allocate  process i's
            PFORCES[i] array in its local memory */
        
        PFORCES = (double ****) G_MALLOC(NumProcs * sizeof (double ***));
        { int i,j,k;
          
          for (i = 0; i < NumProcs; i++) {
              PFORCES[i] = (double ***) G_MALLOC(NMOL * sizeof (double **));
              for (j = 0; j < NMOL; j++) {
                  PFORCES[i][j] = (double **) G_MALLOC(NDIR * sizeof (double *));
                  for (k = 0; k < NDIR; k++) {
                      PFORCES[i][j][k] = (double *) G_MALLOC(NATOM * sizeof (double));
                  }
              }
          }
      }
        /* macro calls to initialize synch varibles  */
        
        BARINIT(gl->start);
        BARINIT(gl->InterfBar);
        BARINIT(gl->PotengBar);
        LOCKINIT(gl->IOLock);
        LOCKINIT(gl->IndexLock);
        LOCKINIT(gl->IntrafVirLock);
        LOCKINIT(gl->InterfVirLock);
        LOCKINIT(gl->FXLock);
        LOCKINIT(gl->FYLock);
        LOCKINIT(gl->FZLock);
        if (NMOL < MAXLCKS) {
            ALOCKINIT(gl->MolLock, NMOL);
        }
        else {
            ALOCKINIT(gl->MolLock, MAXLCKS);
        }
        LOCKINIT(gl->KinetiSumLock);
        LOCKINIT(gl->PotengSumLock);
        
        /* set up control for static scheduling */
        
        MolsPerProc = NMOL/NumProcs;
        StartMol[0] = 0;
        for (pid = 1; pid < NumProcs; pid += 1) {
            StartMol[pid] = StartMol[pid-1] + MolsPerProc;
        }
        StartMol[NumProcs] = NMOL;
    }
    
    SYSCNS();    /* sub. call to initialize system constants  */
    
    fprintf(six,"\nTEMPERATURE                = %8.2f K\n",TEMP);
    fprintf(six,"DENSITY                    = %8.5f G/C.C.\n",RHO);
    fprintf(six,"NUMBER OF MOLECULES        = %8d\n",NMOL);
    fprintf(six,"NUMBER OF PROCESSORS       = %8d\n",NumProcs);
    fprintf(six,"TIME STEP                  = %8.2e SEC\n",TSTEP);
    fprintf(six,"ORDER USED TO SOLVE F=MA   = %8d \n",NORDER);
    fprintf(six,"NO. OF TIME STEPS          = %8d \n",NSTEP);
    fprintf(six,"FREQUENCY OF DATA SAVING   = %8d \n",NSAVE);
    fprintf(six,"FREQUENCY TO WRITE RST FILE= %8d \n",NRST);
    fprintf(six,"SPHERICAL CUTOFF RADIUS    = %8.4f ANGSTROM\n",CUTOFF);
    fflush(six);
    
    
    /* initialization routine; also reads displacements and
       sets up random velocities*/
    INITIA();
    
    /*.....start molecular dynamic loop */
    
    gl->tracktime = 0;
    gl->intratime = 0;
    gl->intertime = 0;
    
    /* initialize Index to 1 so that the first created child gets 
       id 1, not 0 */
    
    gl->Index = 1;
    
    /* spawn helper processes, each getting its unique process id */
    CLOCK(gl->createstart);
    //for (ProcID = 1; ProcID < NumProcs; ProcID += 1) {
    CREATE(WorkStart, NumProcs);
    //}
    CLOCK(gl->createend);
    LOCK(gl->IOLock);
    printf("CREATETIME = %d\n",gl->createend-gl->createstart);
    UNLOCK(gl->IOLock);
    
    if (NSAVE > 0)  /* not true for input decks provided */
        fprintf(six,"COLLECTING X AND V DATA AT EVERY %4d TIME STEPS \n",NSAVE);
    
    /* call routine to do the timesteps, with own id passed as 0 */
    CLOCK(gl->computestart);
    
    //XTT = MDMAIN(NFSV,NFRST,NSTEP,NRST,NPRINT,NSAVE,LKT,NORD1,0); 
    
    /* macro to make main process wait for all others to finish */
    WAIT_FOR_END(NumProcs)
        CLOCK(gl->computeend);
    
    printf("COMPUTESTART (after initialization) = %lu\n",gl->computestart);
    printf("COMPUTEEND = %lu\n",gl->computeend);
    printf("COMPUTETIME (after initialization) = %lu\n",gl->computeend-gl->computestart);
    printf("Measured Time (2nd timestep onward) = %lu\n",gl->tracktime);
    printf("Intramolecular time only (2nd timestep onward) = %lu\n",gl->intratime);
    printf("Intermolecular time only (2nd timestep onward) = %lu\n",gl->intertime);
    printf("Other time (2nd timestep onward) = %lu\n",gl->tracktime - gl->intratime - gl->intertime);
    
    printf("\nExited Happily with XTT = %g (note: XTT value is garbage if NPRINT > NSTEP)\n", XTT);
    
    MAIN_END;
} /* main.c */

void WorkStart() /* routine that each created process starts at;
                    it simply calls the timestep routine */
{
    unsigned ProcID;
    double MDMAIN();
    
    LOCK(gl->IndexLock);
    ProcID = gl->Index++;
    UNLOCK(gl->IndexLock);
    
    ProcID = ProcID % NumProcs; 
    
    /*  POSSIBLE ENHANCEMENT:  Here's where one might bind processes 
        to processors if one wanted to.
        */
    
    MDMAIN(NFSV,NFRST,NSTEP,NRST,NPRINT,NSAVE,LKT,NORD1,ProcID);
}
