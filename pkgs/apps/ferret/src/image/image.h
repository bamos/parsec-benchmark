#ifndef __CASS_IMAGE__
#define __CASS_IMAGE__

#include <cass.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CHAN
#define CHAN 3
#define	MNTS 3
#endif

int image_init (const char *path);
int image_cleanup (void);

int image_ping (const char *filename, int *width, int *height);
int image_read_rgb (const char *filename, int *width, int *height, unsigned char **data);
int image_read_hsv (const char *filename, int *width, int *height, unsigned char **data);

int image_read_rgb_hsv (const char *filename, int *width, int *height, unsigned char **rgb, unsigned char **hsv);

int image_read_gray (const char *filename, int *width, int *height, float **data);
int image_write_rgb (const char *filename, int width, int height, unsigned char *data);

/* dataset has only 1 vecset */
int image_extract_helper (unsigned char *HSV, unsigned char *mask, int width, int height, int nrgn, cass_dataset_t *ds);
//int image_extract (const char *filename, cass_dataset_t *dataset);

int image_segment (void **output, int *num_ccs, void *pixels, int width, int height);
/* for feature extraction */

void add_edge(unsigned char *rgb, int nx, int ny, unsigned char *rmap);

int image_sift_extract (const char *fname, cass_dataset_t *ds, int O, int S, int o_min);

void add_edge(unsigned char *rgb, int nx, int ny, unsigned char *rmap);

typedef struct {
	int		a1, b1, a2, b2; 
	int		u1, v1, u2, v2; 
	int		cx, cy; 
	int		p1, p2; 
} box_t; 

typedef struct {
	int		nbox; 
	int		size; 
	box_t		**boxes;
} box_set_t; 

typedef struct {
	int		nrow, ncol, nrgn, size;
	uchar		**map;
	box_set_t	*box_set; 
	int		*rgn_sz;
	float		*rgn[CHAN][MNTS]; 
} img_map_t;

box_t *box_new(int a, int b); 

void box_insert_pxl(box_t *box, int a, int b, int clear); 

void box_to_vec(box_t *box, float *vec, int offset);

box_set_t *box_set_new(int hint);

void box_set_free(box_set_t **box_set);

void box_set_insert_pxl(box_set_t *box_set, int r, int a, int b); 

void img_map_free(img_map_t **img_map); 

#ifdef __cplusplus
};
#endif

#endif

