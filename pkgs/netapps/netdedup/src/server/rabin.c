#include <assert.h>
#include <stdlib.h>
#include <stdint.h>

#include "dedupdef.h"
#include "rabin.h"

#undef PRINT

/* Functions to compute rabin fingerprints */

//u32int *rabintab = 0;
//u32int *rabinwintab = 0;
//static u32int irrpoly = 0x759ddb9f;
static u32int irrpoly = 0x45c2b6a1;

uint32_t bswap32(x) uint32_t x; {
  return  ((x << 24) & 0xff000000 ) |
          ((x <<  8) & 0x00ff0000 ) |
          ((x >>  8) & 0x0000ff00 ) |
          ((x >> 24) & 0x000000ff );
}

static u32int fpreduce(u32int x, u32int irr) {
  int i;

  for(i=32; i!=0; i--){
    if(x >> 31){
      x <<= 1;
      x ^= irr;
    }else
      x <<= 1;
  }
  return x;
}

static void fpmkredtab(u32int irr, int s, u32int *tab) {
  u32int i;

  for(i=0; i<256; i++)
    tab[i] = fpreduce(i<<s, irr);
  return;
}

static u32int fpwinreduce(u32int irr, int winlen, u32int x, u32int * rabintab) {
  int i;
  u32int winval;

  winval = 0;
  winval = ((winval<<8)|x) ^ rabintab[winval>>24];
  for(i=1; i<winlen; i++)
    winval = (winval<<8) ^ rabintab[winval>>24];
  return winval;
}

static void fpmkwinredtab(u32int irr, int winlen, u32int * rabintab, u32int *rabinwintab) {
  u32int i;

  for(i=0; i<256; i++)
          rabinwintab[i] = fpwinreduce(irr, winlen, i, rabintab);
  return;
}

void rabininit(int winlen, u32int * rabintab, u32int * rabinwintab) {
  //rabintab = malloc(256*sizeof rabintab[0]);
  //rabinwintab = malloc(256*sizeof rabintab[0]);
  fpmkredtab(irrpoly, 0, rabintab);
  fpmkwinredtab(irrpoly, winlen, rabintab, rabinwintab);
  return;
}

int rabinseg(uchar *p, int n, int winlen, u32int * rabintab, u32int * rabinwintab) {
  int i;
  u32int h;
  u32int x;

  USED(winlen);
  if(n < NWINDOW)
    return n;

  h = 0;
  for(i=0; i<NWINDOW; i++){
    x = h >> 24;
    h = (h<<8)|p[i];
    h ^= rabintab[x];
  }
  if((h & RabinMask) == 0)
    return i;
  while(i<n){
    x = p[i-NWINDOW];
    h ^= rabinwintab[x];
    x = h >> 24;
    h <<= 8;
    h |= p[i++];
    h ^= rabintab[x];
    if((h & RabinMask) == 0)
      return i;
  }
  return n;
}

