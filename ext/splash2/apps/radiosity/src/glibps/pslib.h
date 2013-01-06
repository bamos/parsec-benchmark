/* -*-mode:c-*- */
/**************************************************************
*
*        CS348C   Radiosity
*
*        Device independent graphics package.
*
*        May 6, 1991
*                                      Tsai, Tso-Sheng
*                                      Totsuka, Takashi
*
***************************************************************/

#ifndef _PSLIB_H
#define _PSLIB_H


/****************************************
*
*    Library function type definition
*
*****************************************/

extern void ps_open() ;
extern void ps_close() ;
extern void ps_linewidth() ;
extern void ps_line() ;
extern void ps_polygonedge() ;
extern void ps_polygon() ;
extern void ps_spolygon() ;
extern void ps_clear() ;
extern void ps_setup_view() ;



#endif
