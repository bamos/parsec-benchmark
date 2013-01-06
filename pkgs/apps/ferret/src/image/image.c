/* AUTORIGHTS
Copyright (C) 2007 Princeton University
      
This file is part of Ferret Toolkit.

Ferret Toolkit is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <jpeglib.h>
#include "image.h"

#define DEFAULT_SIZE          128
#define EPSILON               1e-6F
#define RESIZE_FILTER_SUPPORT 3.0F

static inline float sinc(float x)
{
  if (x == 0.0)
    return(1.0);
  return(sin(M_PI*(double)x)/(M_PI*(double) x));
}

static inline float weight(float x)
{
  x=fabs(x);
  return(sinc(x/RESIZE_FILTER_SUPPORT)*sinc(x));
}

static inline double Max(double x,double y)
{
    return x > y ? x : y;
}

static inline double Min(double x,double y)
{
    return y > x ? x : y;
}

static inline unsigned char myround (float v) {
    if (v <= 0) return 0;
    if (v >= 255) return 255;
    return v + 0.5;
}

int horizontal(const unsigned char *image, int orig_width, int orig_height, unsigned char *resize_image, int width)
{
  float *contrib;
  float factor=(float)width/(float)orig_width;
  float scale=Max(1.0/factor,1.0);
  float support=scale*RESIZE_FILTER_SUPPORT;
  long x;
  if (support < 0.5) // sampling
  {
    support=(float) 0.5;
    scale=1.0;
  }
  contrib=(float *)malloc((size_t) (2.0*support+3.0) * sizeof(float));
  if (contrib == NULL) fatal("out of memory");
  scale=1.0/scale;
  for (x=0; x < (long) width; x++)
  {
    long i, n, start, stop;
    float center, density;
    register long y;
    center=(float) (x+0.5)/factor;
    start=(long) (Max(center-support-EPSILON,0.0)+0.5);
    stop=(long) (Min(center+support,(double) orig_width)+0.5);
    density=0.0;
    for (n=0; n < (stop-start); n++)
    {
      contrib[n]=weight(scale*((float)(start+n)-center+0.5));
      density+=contrib[n];
    }
    for (i=0; i < n; i++) {
          contrib[i]/=density;
    }
    for (y=0; y < (long) orig_height; y++)
    {
        const unsigned char *p = image + CHAN * (y * orig_width + start);
        unsigned char *q = resize_image + CHAN * (y * width + x);
        float r = 0, g = 0, b = 0;
          for (i=0; i < n; i++)
          {
            float alpha =contrib[i];
            r += alpha * *p++;
            g += alpha * *p++;
            b += alpha * *p++;
          }
          *q++ = myround(r);
          *q++ = myround(g);
          *q++ = myround(b);
    }
  }
  free(contrib);
  return 0;
}

int vertical(const unsigned char *image, int orig_width, int orig_height, unsigned char *resize_image, int height) {
  long y;
  float *contrib;
  float factor=(float)height/(float)orig_height;
  float scale=Max(1.0/factor,1.0);
  float support=scale*RESIZE_FILTER_SUPPORT;
  if (support < 0.5) // sampling
  {
    support=(float) 0.5;
    scale=1.0;
  }
  contrib=(float *)malloc((size_t) (2.0*support+3.0) * sizeof(float));
  if (contrib == NULL) fatal("out of memory");
  scale=1.0/scale;
  for (y=0; y < (long) height; y++)
  {
    long i, n, start, stop;
    float center, density;
    register long x;
    center=(float) (y+0.5)/factor;
    start=(long) (Max(center-support-EPSILON,0.0)+0.5);
    stop=(long) (Min(center+support,(double) orig_height)+0.5);
    density=0.0;
    for (n=0; n < (stop-start); n++)
    {
      contrib[n]=weight(scale*((float)(start+n)-center+0.5));
      density+=contrib[n];
    }
    for (i=0; i < n; i++) {
          contrib[i]/=density;
    }
    for (x=0; x < (long) orig_width; x++)
    {
        const unsigned char *p = image + CHAN * (start * orig_width + x);
        unsigned char *q = resize_image + CHAN * (y * orig_width + x);
        float r = 0, g = 0, b = 0;
          for (i=0; i < n; i++)
          {
            float alpha =contrib[i];
            r += alpha * *p;
            g += alpha * *(p+1);
            b += alpha * *(p+2);
            p += orig_width * CHAN;
          }
          *q++ = myround(r);
          *q++ = myround(g);
          *q++ = myround(b);
    }
  }
  free(contrib);
  return 0;
}

unsigned char *resize (unsigned char *image, int orig_width, int orig_height, int width, int height) 
{
  unsigned char *filter_image, *resize_image;
  if (width * orig_height > height * orig_width) {
    filter_image=(unsigned char *)malloc(width * orig_height * CHAN);
    resize_image=(unsigned char *)malloc(width * height * CHAN);
    if ((filter_image == NULL) || (resize_image == NULL)) fatal("out of memory");
    horizontal(image, orig_width, orig_height, filter_image, width);
    vertical(filter_image, width, orig_height, resize_image, height);
  }
  else {
    filter_image=(unsigned char *)malloc(orig_width * height * CHAN);
    resize_image=(unsigned char *)malloc(width * height * CHAN);
    if ((filter_image == NULL) || (resize_image == NULL)) fatal("out of memory");
    vertical(image, orig_width, orig_height, filter_image, height);
    horizontal(filter_image, orig_width, height, resize_image, width);
  }
  free(filter_image);
  return resize_image;
}


int image_init (const char *path)
{
    return 0;
}

int image_cleanup (void)
{
    return 0;
}

void pixel_rgb2hsv (const unsigned char *rgb, unsigned char *hsv)
{
     unsigned char r = rgb[0];
     unsigned char g = rgb[1];
     unsigned char b = rgb[2];
     float h = 0, s = 0, v = 0;
     unsigned char delta = 0;
     unsigned char mn = r, mx = r;
 
     hsv[0] = hsv[1] = hsv[2] = 0;
 
     if (g > mx) { mx = g; }
     if (g < mn) { mn = g; }
     if (b > mx) { mx = b;}
     if (b < mn) { mn = b;}

     delta = mx - mn;
 
     hsv[2] = mx;       // V

     if (mx == 0) return;

     hsv[1] = (unsigned)delta * 255 / (unsigned)mx;
 
     if (delta == 0) return;

     float hue = 0;
     if (mx == r) {
         hue = ((float)g - (float)b) / (float)delta;
     }
     else if (mx == g) {
         hue = 2.0 + ((float)b - (float)r) / (float)delta;
     }
     else {
         hue = 4.0 + ((float)r - (float)g) / (float)delta;
     }
     if (hue < 0) hue += 6.0;
     hsv[0] = 255 * hue / 6.0;
}
 
void pixel_hsv2rgb (const unsigned char *hsv, unsigned char *rgb)
{
     unsigned char h = hsv[0];
     unsigned char s = hsv[1];
     unsigned char v = hsv[2];
     rgb[0] = rgb[1] = rgb[2];

     if (s == 0) {
         rgb[0] = rgb[1] = rgb[2] = v;
     }

     float hue = h * 6.0 / 255;
     float f = hue - floor(hue);
     unsigned p = v * (255.0 - s) / 255;
     unsigned q = v * (255.0 - s * f) / 255;
     unsigned t = v * (255.0 - s * (1.0 - f)) / 255;
     switch ((int)hue) {
        case 0:
        default:
            rgb[0] = v;
            rgb[1] = t;
            rgb[2] = p;
            break;
        case 1:
            rgb[0] = q;
            rgb[1] = v;
            rgb[2] = p;
            break;
        case 2:
            rgb[0] = p;
            rgb[1] = v;
            rgb[2] = t;
            break;
        case 3:
            rgb[0] = p;
            rgb[1] = q;
            rgb[2] = v;
            break;
        case 4:
            rgb[0] = t;
            rgb[1] = p;
            rgb[2] = v;
            break;
        case 5:
            rgb[0] = v;
            rgb[1] = p;
            rgb[2] = q;
            break;
     }
}


void rgb2hsv (const unsigned char *rgb, int width, int height, unsigned char *hsv) {
 int i;
 for (i = 0; i < width * height; i++) {
    pixel_rgb2hsv(rgb, hsv);
    rgb += CHAN;
    hsv += CHAN;
 }
}

void hsv2rgb (const unsigned char *hsv, int width, int height, unsigned char *rgb) {
 int i;
 for (i = 0; i < width * height; i++) {
    pixel_hsv2rgb(hsv, rgb);
    rgb += CHAN;
    hsv += CHAN;
 }
}
 

/*
 * Sample routine for JPEG decompression.  We assume that the source file name
 * is passed in.  We want to return 1 on success, 0 on error.
 */

int image_read_rgb_hsv (const char *filename, int *width, int *height, unsigned char **data_rgb, unsigned char **data_hsv)
{
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
  FILE * infile;        /* source file */
  unsigned char *orig;
  unsigned char *rgb;        /* Output row buffer */
  unsigned char *hsv;
  JSAMPROW row_pointer[1];  /* pointer to JSAMPLE row[s] */
  int row_stride;       /* physical row width in output buffer */
  if ((infile = fopen(filename, "rb")) == NULL) {
    fprintf(stderr, "can't open %s\n", filename);
    return 1;
  }
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);
  jpeg_stdio_src(&cinfo, infile);
  (void) jpeg_read_header(&cinfo, TRUE);
  (void) jpeg_start_decompress(&cinfo);
  row_stride = cinfo.output_width * cinfo.output_components;
  orig = (unsigned char *)malloc(cinfo.output_width * cinfo.output_height * cinfo.output_components);
  if (orig == NULL) fatal("out of memory");
  row_pointer[0] = orig;
  while (cinfo.output_scanline < cinfo.output_height) {
    (void) jpeg_read_scanlines(&cinfo, row_pointer, 1);
    row_pointer[0] += row_stride;
  }
  (void) jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);
  fclose(infile);

  rgb = resize(orig, cinfo.output_width, cinfo.output_height, DEFAULT_SIZE, DEFAULT_SIZE);
  hsv = (unsigned char *)malloc(DEFAULT_SIZE * DEFAULT_SIZE * CHAN);
  rgb2hsv(rgb, DEFAULT_SIZE, DEFAULT_SIZE, hsv);

  free(orig);

  *width = DEFAULT_SIZE;
  *height = DEFAULT_SIZE;
  *data_rgb = rgb;
  *data_hsv = hsv;
  return 0;
}

int image_write_rgb (const char *filename, int width, int height, unsigned char *data)
{
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  FILE * outfile;       /* target file */
  JSAMPROW row_pointer[1];  /* pointer to JSAMPLE row[s] */
  int row_stride;       /* physical row width in image buffer */

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);
  if ((outfile = fopen(filename, "wb")) == NULL) {
    fprintf(stderr, "can't open %s\n", filename);
    exit(1);
  }
  jpeg_stdio_dest(&cinfo, outfile);
  cinfo.image_width = width;
  cinfo.image_height = height;
  cinfo.input_components = 3;
  cinfo.in_color_space = JCS_RGB;
  jpeg_set_defaults(&cinfo);
  jpeg_start_compress(&cinfo, TRUE);
  row_stride = width * 3;
  row_pointer[0] = data;
  while (cinfo.next_scanline < cinfo.image_height) {
    (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
    row_pointer[0] += row_stride;
  }
  jpeg_finish_compress(&cinfo);
  fclose(outfile);
  jpeg_destroy_compress(&cinfo);
}

