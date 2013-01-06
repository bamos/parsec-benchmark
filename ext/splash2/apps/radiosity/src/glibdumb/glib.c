/* -*-mode:c-*- */
/************************************************************************
*
*        CS348C   Radiosity
*
*	(Dirty) extension to YOSSI interface.
*
*
*       May 6, 1991
*                                      Tsai, Tso-Sheng
*                                      Totsuka, Takashi
*
*       Derived from xsupport.c by Yossi Friedman
*
*************************************************************************/

#include "glib.h"
/*** Data structure used in radiosity algorithm ***/

typedef struct {
      float x, y, z ;
} Point ;

typedef struct {
      float r, g, b ;
} Rgb ;



/**************************************************
*
*    Globals
*
***************************************************/



/**************************************************
*
*    g_init()
*
***************************************************/

void g_init( ac, av )

    int ac ;
    char *av[] ;
{
}



/**************************************************
*
*    g_start()
*
***************************************************/

void g_start( mouse_func, n_sliders, slider_def, n_choices, choice_def )

    void (*mouse_func)() ;
    int n_sliders ;
    slider *slider_def ;
    int n_choices ;
    choice *choice_def ;
{
}


/**************************************************
*
*    g_color()
*
***************************************************/

void g_color( color )

    int color ;
{
}

void g_rgb( color )

    Rgb color ;
{
}


/**************************************************
*
*    g_line()
*
***************************************************/

void g_line( p1, p2 )

    Point *p1, *p2 ;
{
}


/**************************************************
*
*    g_polygon()
*
***************************************************/

void g_polygon( n, p_list )

    int n ;
    Point *p_list ;
{
}


/**************************************************
*
*    g_spolygon()
*
***************************************************/

void g_spolygon( n, p_list, c_list )

    int n ;
    Point *p_list ;
    Rgb   *c_list ;
{
}


/**************************************************
*
*    g_clear()
*
***************************************************/

void g_clear()
{
}



/**************************************************
*
*    g_setup_view()
*
***************************************************/

void g_setup_view( rot_x, rot_y, dist, zoom )

    float rot_x, rot_y, dist, zoom ;
{
}



/**************************************************
*
*    g_get_screen_size()
*
***************************************************/

void g_get_screen_size( u, v )

    int *u, *v ;
{
}


/**************************************************
*
*    g_flush()
*
***************************************************/

void g_flush()
{
}


