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

/* shared memory implementation of the multigrid method
   implementation uses red-black gauss-seidel relaxation
   iterations, w cycles, and the method of half-injection for
   residual computation */ 

#include <stdio.h>
#include <math.h>
#include <time.h>
#include "decs.h"

void multig(int);
void relax(int,double *,int,int);
void rescal(int,int);
void intadd(int,int);
void putz(int,int);

void multig(my_id)

/* perform multigrid (w cycles)                                     */

int my_id;

{
   int iter;
   double wu; 
   double errp;
   int m;
   int minlevel;
   int flag1;
   int flag2;
   int k;
   int my_num;
   double wmax;
   double local_err;
   double red_local_err;
   double black_local_err;
   double g_error;
   int i;
   int j;

   flag1 = 0;
   flag2 = 0;
   iter = 0;
   m = numlev-1;
   wmax = maxwork;
   minlevel = minlev;
   my_num = my_id;
   wu = 0.0;
   
   k = m;
   g_error = 1.0e30;
   while ((!flag1) && (!flag2)) {
     errp = g_error;
     iter++;
     if (my_num == MASTER) {
       multi->err_multi = 0.0;
     }

/* barrier to make sure all procs have finished intadd or rescal   */
/* before proceeding with relaxation                               */

     BARRIER(bars->error_barrier,nprocs)  

     relax(k,&red_local_err,RED_ITER,my_num);

/* barrier to make sure all red computations have been performed   */

     BARRIER(bars->error_barrier,nprocs)  

     relax(k,&black_local_err,BLACK_ITER,my_num);

/* compute max local error from red_local_err and black_local_err  */

     if (red_local_err > black_local_err) {
       local_err = red_local_err;
     } else {
       local_err = black_local_err;
     }

/* update the global error if necessary                            */

     LOCK(locks->error_lock)
     if (local_err > multi->err_multi) {
       multi->err_multi = local_err;
     }
     UNLOCK(locks->error_lock)

/* a single relaxation sweep at the finest level is one unit of    */
/* work                                                            */

     wu+=pow((double)4.0,(double)k-m);

/* barrier to make sure all processors have checked local error    */

     BARRIER(bars->error_barrier,nprocs)  

     g_error = multi->err_multi;

/* barrier to make sure master does not cycle back to top of loop  */
/* and reset global->err before we read it and decide what to do   */

     BARRIER(bars->error_barrier,nprocs)  

     if (g_error >= lev_tol[k]) {
       if (wu > wmax) {
/* max work exceeded                                               */
         flag1 = 1;
         fprintf(stderr,"ERROR: Maximum work limit %0.5f exceeded\n",wmax);
         exit(-1);
       } else {
/* if we have not converged                                        */
         if ((k != 1) && (g_error/errp >= 0.6) && (k > minlevel)) {  
/* if need to go to coarser grid                                   */
           rescal(k,my_num);
/* transfer residual to rhs of coarser grid                        */
           lev_tol[k-1] = 0.3 * g_error;
           k = k-1;
           putz(k,my_num);
/* make initial guess on coarser grid zero                         */
           g_error = 1.0e30;
         }
       }
     } else {
/* if we have converged at this level                              */
       if (k == m) {
/* if finest grid, we are done                                     */
         flag2 = 1;
       } else {
/* else go to next finest grid                                     */
         intadd(k,my_num);
         k++;
         g_error = 1.0e30;
       }
     }
   }
   if (do_output) {
     if (my_num == MASTER) {
       printf("iter %d, level %d, residual norm %12.8e, work = %7.3f\n",
               iter,k,multi->err_multi,wu);
     }
   }
}

void relax(k,err,color,my_num) 

int k;
double *err;
int color;
int my_num;

/* perform red or black iteration (not both)                    */

{
   int i;
   int j;
   int istart;
   int jstart;
   int iend;
   int jend;
   int oddistart;
   int oddjstart;
   int evenistart;
   int evenjstart;
   int oddiendst;
   int eveniendst;
   int oddjendst;
   int evenjendst;
   int tiend;
   int tjend;
   double a;
   double h;
   double factor;
   double maxerr;
   double newerr;
   double oldval;
   double newval;
   double f1;
   double f2;
   double f3;
   double f4;
   double ressqr;

   i = 0;
   j = 0;

   *err = 0.0;
   h = lev_res[k];

/* points whose sum of row and col index is even do a red iteration, */
/* others do a black				                     */

   evenistart = gp[my_num].eist[k];
   evenjstart = gp[my_num].ejst[k];
   oddistart = gp[my_num].oist[k];
   oddjstart = gp[my_num].ojst[k];
   eveniendst = gp[my_num].eiest[k];
   evenjendst = gp[my_num].ejest[k];
   oddiendst = gp[my_num].oiest[k];
   oddjendst = gp[my_num].ojest[k];

   iend = gp[my_num].rel_start_y[k] + gp[my_num].rel_num_y[k];
   jend = gp[my_num].rel_start_x[k] + gp[my_num].rel_num_x[k];

   factor = 4.0 - eig2 * h * h ;
   maxerr = 0.0;
   if (color == RED_ITER) {
     for (i=evenistart;i<iend;i+=2) {
       for (j=evenjstart;j<jend;j+=2) {
         a = multi->q_multi[k][i][j+1] + multi->q_multi[k][i][j-1] + 
	     multi->q_multi[k][i-1][j] + multi->q_multi[k][i+1][j] - 
	     multi->rhs_multi[k][i][j] ;
         oldval = multi->q_multi[k][i][j];
         newval = a / factor;
         newerr = oldval - newval;
         multi->q_multi[k][i][j] = newval;
         if (fabs(newerr) > maxerr) {
           maxerr = fabs(newerr);
         }
       }
     }
     for (i=oddistart;i<iend;i+=2) {
       for (j=oddjstart;j<jend;j+=2) {
         a = multi->q_multi[k][i][j+1] + multi->q_multi[k][i][j-1] + 
	     multi->q_multi[k][i-1][j] + multi->q_multi[k][i+1][j] - 
	     multi->rhs_multi[k][i][j] ;
         oldval = multi->q_multi[k][i][j];
         newval = a / factor;
         newerr = oldval - newval;
         multi->q_multi[k][i][j] = newval;
         if (fabs(newerr) > maxerr) {
           maxerr = fabs(newerr);
         }
       }
     }
   } else if (color == BLACK_ITER) {
     for (i=evenistart;i<iend;i+=2) {
       for (j=oddjstart;j<jend;j+=2) {
         a = multi->q_multi[k][i][j+1] + multi->q_multi[k][i][j-1] + 
	     multi->q_multi[k][i-1][j] + multi->q_multi[k][i+1][j] - 
	     multi->rhs_multi[k][i][j] ;
         oldval = multi->q_multi[k][i][j];
         newval = a / factor;
         newerr = oldval - newval;
         multi->q_multi[k][i][j] = newval;
         if (fabs(newerr) > maxerr) {
           maxerr = fabs(newerr);
         }
       }
     }
     for (i=oddistart;i<iend;i+=2) {
       for (j=evenjstart;j<jend;j+=2) {
         a = multi->q_multi[k][i][j+1] + multi->q_multi[k][i][j-1] + 
	     multi->q_multi[k][i-1][j] + multi->q_multi[k][i+1][j] - 
	     multi->rhs_multi[k][i][j] ;
         oldval = multi->q_multi[k][i][j];
         newval = a / factor;
         newerr = oldval - newval;
         multi->q_multi[k][i][j] = newval;
         if (fabs(newerr) > maxerr) {
           maxerr = fabs(newerr);
         }
       }
     }
   } 
   *err = maxerr;
}

void rescal(kf,my_num)

/* perform half-injection to next coarsest level                */

int kf;
int my_num;

{
   int ic;
   int if17;
   int jf;
   int jc;
   int krc;
   int istart;
   int iend;
   int jstart;
   int jend;
   int oddiendst;
   int eveniendst;
   int oddjendst;
   int evenjendst;
   double hf;
   double hc;
   double s;
   double s1;
   double s2;
   double s3;
   double s4;
   double factor;
   double h;
   double int1;
   double int2;
   double i_int_factor;
   double j_int_factor;
   double int_val;

   krc = kf - 1;
   hc = lev_res[krc];
   hf = lev_res[kf];

   istart = gp[my_num].rlist[krc];
   jstart = gp[my_num].rljst[krc];
   iend = gp[my_num].rlien[krc];
   jend = gp[my_num].rljen[krc];
   iend = gp[my_num].rel_start_y[krc] + gp[my_num].rel_num_y[krc] - 1;
   jend = gp[my_num].rel_start_x[krc] + gp[my_num].rel_num_x[krc] - 1;

   factor = 4.0 - eig2 * hf * hf;

   if17=2*(istart-1);  
   for(ic=istart;ic<=iend;ic++) {
     if17+=2;
     i_int_factor = ic * i_int_coeff[krc] * 0.5;
     jf = 2 * (jstart - 1);
     for(jc=jstart;jc<=jend;jc++) {
       jf+=2;
       j_int_factor = jc*j_int_coeff[krc] * 0.5;
/* method of half-injection uses 2.0 instead of 4.0 */
       s = multi->q_multi[kf][if17][jf+1] + multi->q_multi[kf][if17][jf-1] + 
           multi->q_multi[kf][if17-1][jf] + multi->q_multi[kf][if17+1][jf];
       s1 = 2.0 * (multi->rhs_multi[kf][if17][jf] - s +
		   factor * multi->q_multi[kf][if17][jf]); 
       if ((if17 == 2) || (jf ==2)) {
         s2 = 0;
       } else {
         s = multi->q_multi[kf][if17][jf-1] + multi->q_multi[kf][if17][jf-3] + 
             multi->q_multi[kf][if17-1][jf-2] + multi->q_multi[kf][if17+1][jf-2];
         s2 = 2.0 * (multi->rhs_multi[kf][if17][jf-2] - s +
	  	   factor * multi->q_multi[kf][if17][jf-2]); 
       }
       if ((if17 == 2) || (jf ==2)) {
         s3 = 0;
       } else {
         s = multi->q_multi[kf][if17-2][jf+1] + multi->q_multi[kf][if17-2][jf-1] + 
             multi->q_multi[kf][if17-3][jf] + multi->q_multi[kf][if17-1][jf];
         s3 = 2.0 * (multi->rhs_multi[kf][if17-2][jf] - s +
		     factor * multi->q_multi[kf][if17-2][jf]); 
       }
       if ((if17 == 2) || (jf ==2)) {
         s4 = 0;
       } else {
         s = multi->q_multi[kf][if17-2][jf-1] + multi->q_multi[kf][if17-2][jf-3] + 
         multi->q_multi[kf][if17-3][jf-2] + multi->q_multi[kf][if17-1][jf-2];
         s4 = 2.0 * (multi->rhs_multi[kf][if17-2][jf-2] - s +
		   factor * multi->q_multi[kf][if17-2][jf-2]); 
       }
       int1 = j_int_factor*s4 + (1.0-j_int_factor)*s3;
       int2 = j_int_factor*s2 + (1.0-j_int_factor)*s1;
       int_val = i_int_factor*int1+(1.0-i_int_factor)*int2;
       multi->rhs_multi[krc][ic][jc] = i_int_factor*int1+(1.0-i_int_factor)*int2;  
     }
   }
}

void intadd(kc,my_num)

/* perform interpolation and addition to next finest grid       */

int kc;
int my_num;

{
   int ic;
   int if17;
   int jf;
   int jc;
   int i;
   int kf;
   int istart;
   int jstart;
   int iend;
   int jend;
   double a;
   double am;
   double hc;
   double hf;
   int ifine1;
   int ifine2;
   int jfine1;
   int jfine2;
   double int1;
   double int2;
   double i_int_factor1;
   double j_int_factor1;
   double i_int_factor2;
   double j_int_factor2;
   double int_val;

   kf = kc + 1;
   hc = lev_res[kc];
   hf = lev_res[kf];

   istart = gp[my_num].iist[kc];
   jstart = gp[my_num].ijst[kc];
   iend = gp[my_num].iien[kc];
   jend = gp[my_num].ijen[kc];

   istart = gp[my_num].rel_start_y[kc];
   jstart = gp[my_num].rel_start_x[kc];
   iend = gp[my_num].rel_start_y[kc] + gp[my_num].rel_num_y[kc] - 1;
   jend = gp[my_num].rel_start_x[kc] + gp[my_num].rel_num_x[kc] - 1;
   if17 = 2*(istart-1);  
   for(ic=istart;ic<=iend;ic++) {

     if17+=2;
     ifine1 = if17-1;
     ifine2 = if17;
     i_int_factor1= ((imx[kc]-2)-(ic-1)) * (i_int_coeff[kf]);
     i_int_factor2= ic * i_int_coeff[kf];

     jf = 2*(jstart-1); 

     for(jc=jstart;jc<=jend;jc++) {
       jf+=2;
       jfine1 = jf-1;
       jfine2 = jf;
       j_int_factor1= ((jmx[kc]-2)-(jc-1)) * (j_int_coeff[kf]);
       j_int_factor2= jc * j_int_coeff[kf];

       int1 = j_int_factor1*multi->q_multi[kc][ic][jc-1] + 
	      (1.0-j_int_factor1)*multi->q_multi[kc][ic][jc];
       int2 = j_int_factor1*multi->q_multi[kc][ic-1][jc-1] + 
	      (1.0-j_int_factor1)*multi->q_multi[kc][ic-1][jc];
       multi->q_multi[kf][if17-1][jf-1] += i_int_factor1*int2 + 
	      (1.0-i_int_factor1)*int1;
       int2 = j_int_factor1*multi->q_multi[kc][ic+1][jc-1] + 
	      (1.0-j_int_factor1)*multi->q_multi[kc][ic+1][jc];
       multi->q_multi[kf][if17][jf-1] += i_int_factor2*int2 + 
	      (1.0-i_int_factor2)*int1;
       int1 = j_int_factor2*multi->q_multi[kc][ic][jc+1] + 
	      (1.0-j_int_factor2)*multi->q_multi[kc][ic][jc];
       int2 = j_int_factor2*multi->q_multi[kc][ic-1][jc+1] + 
	      (1.0-j_int_factor2)*multi->q_multi[kc][ic-1][jc];
       multi->q_multi[kf][if17-1][jf] += i_int_factor1*int2 + 
	      (1.0-i_int_factor1)*int1;
       int2 = j_int_factor2*multi->q_multi[kc][ic+1][jc+1] + 
	      (1.0-j_int_factor2)*multi->q_multi[kc][ic+1][jc];
       multi->q_multi[kf][if17][jf] += i_int_factor2*int2 + 
	      (1.0-i_int_factor2)*int1;
     }
   }
}

void putz(k,my_num)

/* initialize a grid to zero in parallel                        */

int k;
int my_num;

{
   int i;
   int j;
   int istart;
   int jstart;
   int iend;
   int jend;

   istart = gp[my_num].pist[k];
   jstart = gp[my_num].pjst[k];
   iend = gp[my_num].pien[k];
   jend = gp[my_num].pjen[k];
   for (i=istart;i<=iend;i++) {
     for (j=jstart;j<=jend;j++) {
       multi->q_multi[k][i][j] = 0.0;
     }
   }
}
