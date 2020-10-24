#ifndef __LISP_GC_H__
#define __LISP_GC_H__
#include <lisp/util/dl_list.h>
#include <stddef.h>

typedef enum lisp_gc_color_t
{ 
  LISP_GC_WHITE = 0,
  LISP_GC_GREY = 1,
  LISP_GC_BLACK = 2,
  // todo remove this cases
  LISP_GC_WHITE_ROOT = 3,
  LISP_GC_GREY_ROOT = 4,
  LISP_GC_BLACK_ROOT = 5
} lisp_gc_color_t;

#define LISP_GC_NUM_COLORS 6

typedef struct lisp_gc_collectible_list_t
{
  lisp_dl_list_t objects;
  lisp_gc_color_t color;
  short int is_root;
  struct lisp_gc_collectible_list_t * other_elements;
  struct lisp_gc_collectible_list_t * grey_elements;
  struct lisp_gc_collectible_list_t * to_elements;
} lisp_gc_collectible_list_t;

typedef struct lisp_gc_color_map_t
{
  lisp_gc_collectible_list_t * white;
  lisp_gc_collectible_list_t * grey;
  lisp_gc_collectible_list_t * black;

  lisp_gc_collectible_list_t * white_root;
  lisp_gc_collectible_list_t * grey_root;
  lisp_gc_collectible_list_t * black_root;

} lisp_gc_color_map_t;

typedef struct lisp_collectible_object_t
{
  lisp_gc_collectible_list_t * lst;
} lisp_collectible_object_t;

typedef struct lisp_gc_t
{
  lisp_gc_color_map_t cons_color_map;

  /* cons memory */
  void ** cons_pages;
  void * current_cons_page;
  lisp_dl_list_t recycled_conses;
  size_t cons_page_size;
  size_t num_cons_pages;
  size_t cons_pos;

} lisp_gc_t;

/**
 * Initialize garbage collector object
 */
int lisp_init_gc(lisp_gc_t * gc);

/**
 * Free garbage collector object allocated with lisp_init_gc
 */
int lisp_free_gc(lisp_gc_t * gc);

/**
 * Set the size of the cons pages.
 * @return LISP_OK or LISP_INVALID
 */
int lisp_gc_set_cons_page_size(lisp_gc_t * gc, size_t page_size);

/**
 * Allocate a cons object.
 *
 * @return pointer to lisp_cons_t structure
 */
struct lisp_cons_t * lisp_gc_alloc_cons(lisp_gc_t * gc);

/**
 * Allocate a root cons object.
 *
 * @return pointer to lisp_cons_t structure
 */
struct lisp_cons_t * lisp_gc_alloc_root_cons(lisp_gc_t * gc);

/**
 * free lisp_cons_t object allocated lisp_gc_alloc_cons.
 *
 * @param gc garbage collector object
 * @param cons object to be freed.
 */
void lisp_gc_free_cons(lisp_gc_t * gc, struct lisp_cons_t * cons);

#endif
