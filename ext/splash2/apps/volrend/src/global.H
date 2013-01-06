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

/************************************************************************
*                                                                    	*
*     global.h: global variables common to entire program               *
*                                                                       *
************************************************************************/

#include <stdint.h>


extern int image_section[NI];
extern int voxel_section[NM];

extern PIXEL *image,*image_block,*mask_image_block;
extern int num_nodes,frame;
extern int block_xlen,block_ylen,num_blocks,num_xblocks,num_yblocks;
extern struct GlobalMemory *Global;
extern PIXEL *shd_address;
extern BOOLEAN *sbit_address;
extern long shd_length;

                                /* Option globals                            */
extern BOOLEAN adaptive;        /* YES for adaptive ray tracing, NO if not   */
                                /* Shading parameters of reflective surface: */
extern float density_opacity[MAX_DENSITY+1];	
                                /*   opacity as function of density          */
extern float magnitude_opacity[MAX_MAGNITUDE+1];
                                /*  opacity as function of magnitude         */
extern int density_epsilon;	/*   minimum (density*map_divisor)           */
				/*     (>= MIN_DENSITY)                      */
extern int magnitude_epsilon;	/*   minimum (magnitude*grd_divisor)**2      */
				/*     (> MIN_MAGNITUDE)                     */
extern PIXEL background;	/*   color of background                     */
extern float light[NM];		/*   normalized vector from object to light  */
extern float ambient_color;     /*   color of ambient reflection     */
extern float diffuse_color;    	/*   color of diffuse reflection     */
extern float specular_color;  	/*   color of specular reflection    */
extern float specular_exponent; /*   exponent of specular reflection */
extern float depth_hither;	/*   percentage of full intensity at hither  */
extern float depth_yon;		/*   percentage of full intensity at yon     */
extern float depth_exponent;	/*   exponent of falloff from hither to yon  */
extern float opacity_epsilon;	/*   minimum opacity                         */
				/*     (usually >= MIN_OPACITY,              */
				/*      < MIN_OPACITY during shading shades  */
				/*      all voxels for generation of mipmap) */
extern float opacity_cutoff;	/*   cutoff opacity                          */
				/*     (<= MAX_OPACITY)                      */
extern int highest_sampling_boxlen;
                                /*   highest boxlen for adaptive sampling    */
				/*     (>= 1)                                */
extern int lowest_volume_boxlen;/*   lowest boxlen for volume data           */
				/*     (>= 1)                                */
extern int volume_color_difference;
                        	/*   minimum color diff for volume data      */
				/*     (>= MIN_PIXEL)                        */
extern float angle[NM];         /*  initial viewing angle                    */
extern int pyr_highest_level;	/*   highest level of pyramid to look at     */
				/*     (<= MAX_PYRLEVEL)                     */
extern int pyr_lowest_level;	/*   lowest level of pyramid to look at      */
				/*     (>= 0)                                */

                                /* Pre_View Globals                          */
extern short frust_len; 	/* Size of clipping frustum                  */
				/*   (mins will be 0 in this program,        */
				/*    {x,y}len will be <= IK{X,Y}SIZE)       */
extern float depth_cueing[MAX_OUTLEN];
                         	/* Pre-computed table of depth cueing        */
extern int image_zlen;	       	/* number of samples along viewing ray       */
extern float in_max[NM];        /* Pre-computed clipping aids                */
extern int opc_xlen,opc_xylen;
extern int norm_xlen,norm_xylen;

extern VOXEL *vox_address;
extern short vox_len[NM];
extern long vox_length;
extern int vox_xlen,vox_xylen;


                                /* View Globals                              */
extern float transformation_matrix[4][4];
                                /* current transformation matrix             */
extern float out_invvertex[2][2][2][NM]; 
                                /* Image and object space centers            */
                                /*   of outer voxels in output map           */
extern float uout_invvertex[2][2][2][NM];
                                /* Image and object space vertices           */
                                /*   of output map unit voxel                */

                                /* Render Globals                            */
extern float obslight[NM];	/*   observer transformed light vector       */
extern float obshighlight[NM];	/*   observer transformed highlight vector   */
extern float invjacobian[NM][NM];
                        	/* Jacobian matrix showing object space      */
				/*   d{x,y,z} per image space d{x,y,z}       */
				/*   [0][0] is dx(object)/dx(image)          */
				/*   [0][2] is dz(object)/dx(image)          */
				/*   [2][0] is dx(object)/dz(image)          */
extern float invinvjacobian[NM][NM];
                        	/*   [i][j] = 1.0 / invjacobian[i][j]        */


                                /* Density map globals                       */
extern uint16_t map_len[NM];	/* Size of this density map                  */
extern uint32_t map_length;	/* Total number of densities in map          */
				/*   (= product of lens)                     */
extern DENSITY *map_address;	/* Pointer to map                            */

                                /* Normal and gradient magnitude map globals */
extern uint16_t norm_len[NM];	/* Size of this normal map                   */
extern uint32_t norm_length;	/* Total number of normals in map            */
				/*   (= NM * product of lens)                */
extern NORMAL *norm_address;	/* Pointer to normal map                     */
extern float nmag_epsilon;

                                /* Opacity map globals                       */
extern uint16_t opc_len[NM];	/* Size of this opacity map                  */
extern uint32_t opc_length;	/* Total number of opacities in map          */
				/*   (= product of lens)                     */
extern OPACITY *opc_address;	/* Pointer to opacity map                    */

                                /* Octree globals                            */
extern uint16_t pyr_levels;	/* Number of levels in this pyramid          */
extern uint16_t pyr_len[MAX_PYRLEVEL+1][NM];
                        	/* Number of voxels on each level            */
extern uint16_t pyr_voxlen[MAX_PYRLEVEL+1][NM];
                          	/* Size of voxels on each level              */
extern uint32_t pyr_length[MAX_PYRLEVEL+1];
                                /* Total number of bytes on this level       */
				/* (= (int)((product of lens+7)/8))          */
extern BYTE *pyr_address[MAX_PYRLEVEL+1];
                                /* Pointer to binary pyramid                 */
				/*   (only pyr_levels sets of lens, lengths, */
				/*    and 3-D arrays are written to file)    */
extern long pyr_offset1;	/* Bit offset of desired bit within pyramid  */
extern long pyr_offset2;	/* Bit offset of bit within byte             */
extern BYTE *pyr_address2;	/* Pointer to byte containing bit            */

                                /* Image globals                             */
extern short image_len[NI];     /* Size of image                             */
extern uint32_t image_length;   /* Total number of pixels in map             */
extern PIXEL *image_address;    /* Pointer to image                          */
extern short mask_image_len[NI];/* Size of mask image for adaptive ray trace */
extern uint32_t mask_image_length; /* Total number of pixels in mask image   */
extern MPIXEL *mask_image_address;
                                /* Pointer to image                          */

