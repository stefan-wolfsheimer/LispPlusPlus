#ifndef __LISP_GC_COLOR_H__
#define __LISP_GC_COLOR_H__

#define LISP_GC_NUM_COLORS 3

/**
 * Color for 3-generation garbage collector
 */
typedef enum lisp_gc_color_t
{ 
  LISP_GC_WHITE = 0,
  LISP_GC_GREY  = 1,
  LISP_GC_BLACK = 2,
  LISP_GC_NO_COLOR = 3
} lisp_gc_color_t;

#endif
