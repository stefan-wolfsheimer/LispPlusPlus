#ifndef __LISP_COMPLEX_OBJECT_H__
#define __LISP_COMPLEX_OBJECT_H__
#include "typedefs.h"
#include "gc_color.h"

/**
 * basic complex object
 */
typedef struct lisp_complex_object_t
{
  struct lisp_gc_collectible_list_t * gc_list;
  size_t ref_count;
  lisp_type_id_t type_id;
} lisp_complex_object_t;

/**
 * @todo implement function
 * Get the color of the cons in the 3 generation garbage collector.
 */
lisp_gc_color_t lisp_complex_object_get_color(const lisp_complex_object_t * obj);

/**
 * @todo implement function
 * Return true value if cons is in the root set
 */
short int lisp_complex_object_is_root(const lisp_complex_object_t * obj);

/**
 * @todo implement function
 * If cons is in white list move it to grey list
 */
short int lisp_complex_object_grey(lisp_complex_object_t * obj);

#endif
