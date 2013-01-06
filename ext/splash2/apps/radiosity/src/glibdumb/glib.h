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

#ifndef _GLIB_H
#define _GLIB_H


/****************************************
*
*    Color names
*
*****************************************/

#define G_BLACK   (256)
#define G_RED     (257)
#define G_GREEN   (258)
#define G_YELLOW  (259)
#define G_BLUE    (260)
#define G_MAGENTA (261)
#define G_CYAN    (262)
#define G_WHITE   (263)



/****************************************
*
*    Panel data structures
*
*****************************************/


typedef struct {
    char *name;
    int min, max;
    int init_value;
    int ticks;
    void (*callback)();
} slider;


#define MAX_POSSIBILITIES	32

typedef struct {
    char *name;
    char *possibilities[MAX_POSSIBILITIES];
    int init_value;
    void (*callback)();
} choice;


/****************************************
*
*    Library function type definition
*
*****************************************/

extern void g_init() ;
extern void g_start() ;
extern void g_color(), g_rgb() ;
extern void g_line() ;
extern void g_polygon() ;
extern void g_spolygon() ;
extern void g_clear() ;
extern void g_setup_view() ;
extern void g_get_screen_size() ;
extern void g_flush() ;



#endif
