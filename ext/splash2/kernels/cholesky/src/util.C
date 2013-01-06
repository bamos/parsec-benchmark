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

EXTERN_ENV

#include <math.h>
#include <stdio.h>
#include "matrix.h"

#define Error(m) { printf(m); exit(0); }
#define Outside(M, i) (i >= M.n || i < 0)
#define AddMember(set, new) { int s, n; s = set; n = new; \
			       link[n] = link[s]; link[s] = n; }


int maxm;

SMatrix NewMatrix(n, m, nz)
{
  SMatrix M;

  M.n = n; M.m = m;
  M.col = (int *) MyMalloc((n+1)*sizeof(int), DISTRIBUTED);
  M.startrow = (int *) MyMalloc((n+1)*sizeof(int), DISTRIBUTED);
  M.row = (int *) MyMalloc((m+n)*sizeof(int), DISTRIBUTED);
  if (nz) {
    M.nz = (double *) MyMalloc((m+n)*sizeof(double), DISTRIBUTED);
  }
  else M.nz = NULL;

  if (!M.col || !M.row || (nz && !M.nz)) {
    printf("NewMatrix %d %d: Out of memory\n", n, m);
    exit(0);
  }

  return(M);
}

FreeMatrix(M)
SMatrix M;
{
  MyFree(M.col);
  MyFree(M.startrow);
  MyFree(M.row);
  if (M.nz)
    MyFree(M.nz);
}


double *NewVector(n)
{
  double *v;

  v = (double *) MyMalloc(n*sizeof(double), DISTRIBUTED);

  if (!v && n) {
    printf("Out of memory: NewVector(%d)\n", n);
    exit(0);
  }

  return(v);
}


double Value(i, j, n)
int i, j, n;
{
  if (i == j)
    return((double) maxm+0.1);
  else if (0)
    return(-1.0+(i+j)/(double) n);
  else
    return(-1.0);
}


SMatrix ReadSparse(name, probName)
char *name, *probName;
{
	FILE *fp;
	int n, m, i, j;
	int n_rows, tmp;
	int numer_lines;
	int colnum, colsize, rownum, rowsize;
	char buf[100], type[4];
	SMatrix M, F, LowerToFull();

	if (!name || name[0] == 0)
		fp = stdin;
	else fp = fopen(name, "r");

	if (!fp) 
		Error("Error opening file\n");

	fscanf(fp, "%72c", buf);

	fscanf(fp, "%8c", probName);
	probName[8] = 0;
	DumpLine(fp);

	for (i=0; i<5; i++) {
	  fscanf(fp, "%14c", buf);
	  sscanf(buf, "%d", &tmp);
	  if (i == 3)
	    numer_lines = tmp;
	}
	DumpLine(fp);

	fscanf(fp, "%3c", type);
	type[3] = 0;
	if (!(type[0] != 'C' && type[1] == 'S' && type[2] == 'A')) {
	  fprintf(stderr, "Wrong type: %s\n", type);
	  exit(0);
	}

	fscanf(fp, "%11c", buf); /* pad */

	fscanf(fp, "%14c", buf); sscanf(buf, "%d", &n_rows);

	fscanf(fp, "%14c", buf); sscanf(buf, "%d", &n);

	fscanf(fp, "%14c", buf); sscanf(buf, "%d", &m);

	fscanf(fp, "%14c", buf); sscanf(buf, "%d", &tmp);
	if (tmp != 0)
	  printf("This is not an assembled matrix!\n");
	if (n_rows != n)
	  printf("Matrix is not symmetric\n");
	DumpLine(fp);

	fscanf(fp, "%16c", buf);
	ParseIntFormat(buf, &colnum, &colsize);
	fscanf(fp, "%16c", buf);
	ParseIntFormat(buf, &rownum, &rowsize);
	fscanf(fp, "%20c", buf);
	fscanf(fp, "%20c", buf);
		
	DumpLine(fp); /* format statement */

	M = NewMatrix(n, m, 0);

	ReadVector(fp, n+1, M.col, colnum, colsize);

	ReadVector(fp, m, M.row, rownum, rowsize);

	for (i=0; i<numer_lines; i++) /* dump numeric values */
	  DumpLine(fp);

	for (i=0; i<n; i++)
		ISort(M, i);

	for (i=0; i<=n; i++)
	  M.startrow[i] = M.col[i];

	fclose(fp);

	F = LowerToFull(M);

	maxm = 0;
	for (i=0; i<n; i++)
	  if (F.col[i+1]-F.col[i] > maxm)
	    maxm = F.col[i+1]-F.col[i];

	if (F.nz) {
	  for (j=0; j<n; j++)
	    for (i=F.col[j]; i<F.col[j+1]; i++)
	      F.nz[i] = Value(F.row[i], j, F.n);
	}

	FreeMatrix(M);

	return(F);
}

DumpLine(fp)
FILE *fp;
{
	int c;

	while ((c = fgetc(fp)) != '\n')
		;
}

ParseIntFormat(buf, num, size)
char *buf;
int *num, *size;
{
  char *tmp;

  tmp = buf;

  while (*tmp++ != '(')
    ;
  sscanf(tmp, "%d", num);

  while (*tmp++ != 'I')
    ;
  sscanf(tmp, "%d", size);
}


ReadVector(fp, n, where, perline, persize)
FILE *fp;
int *where;
{
  int i, j, item;
  char tmp, buf[100];

  i = 0;
  while (i < n) {
    fgets(buf, 100, fp);    /* read a line at a time */
    for (j=0; j<perline && i<n; j++) {
      tmp = buf[(j+1)*persize]; buf[(j+1)*persize] = 0;  /* null terminate */
      item = atoi(&buf[j*persize]);
      buf[(j+1)*persize] = tmp;
      where[i++] = item-1;
    }
  }
}


SMatrix LowerToFull(L)
SMatrix L;
{
  SMatrix M;
  int *link, *first;
  int i, j, nextj, ind = 0;

  link = (int *) malloc(L.n*sizeof(int));
  first = (int *) malloc(L.n*sizeof(int));

  for (i=0; i<L.n; i++)
    link[i] = first[i] = -1;
    
  M = NewMatrix(L.n, 2*(L.m-L.n)+L.n, 0);

  for (i=0; i<L.n; i++) {
    M.col[i] = ind;

    for (j=L.col[i]; j<L.col[i+1]; j++)
      if (L.row[j] >= i) {
	M.row[ind++] = L.row[j];
      }

    j = link[i];
    while (j != -1) {
      nextj = link[j];
      M.row[ind++] = j;
      first[j]++;
      if (first[j] < L.col[j+1])
	AddMember(L.row[first[j]], j);
      j = nextj;
    }

    first[i] = L.col[i];
    if (L.row[first[i]] == i)
      first[i]++;
    else {
      fprintf(stderr, "Missing diagonal: %d: ", i);
      for (j=L.col[i]; j<L.col[i+1]; j++)
	fprintf(stderr, "%d ", L.row[j]);
      fprintf(stderr, "\n");
    }

    if (first[i] < L.col[i+1]) {
      AddMember(L.row[first[i]], i);
    }
  }

  M.col[M.n] = ind;

  for (i=0; i<=L.n; i++)
    M.startrow[i] = M.col[i];

  if (ind != M.m)
    printf("Lost some\n");

  free(link); free(first);

  return(M);
}

ISort(M, k)
SMatrix M;
{
	int hi, lo;
	int i, j, tmp;
	double tmp2;

	hi = M.col[k+1];
	lo = M.col[k];

	for (i=lo; i<hi; i++) {
		tmp = M.row[i];
		j = i;
		while (M.row[j-1] > tmp && j > lo) {
			M.row[j] = M.row[j-1];
			j--;
			}
		M.row[j] = tmp;
		}

}
