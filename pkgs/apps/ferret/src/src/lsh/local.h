#ifndef __WDONG_LOCAL__
#define __WDONG_LOCAL__

/* y = a + b x */
static inline void least_squares (double *a, double *b, int n, double sxx, double sxy, double sx, double sy)
{
	*b = (n * sxy - sx*sy) / (n * sxx - sx*sx);
	*a = (sy - *b * sx) / n;
}

int localdim (double a, double b, double *alpha, double *beta, double W, int M, int L, int K);

#endif
