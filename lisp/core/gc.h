#ifndef __LISP_GC_H__
#define __LISP_GC_H__
#include <lisp/util/dl_list.h>
#include <stddef.h>

typedef enum lisp_gc_color_t
{ 
  LISP_GC_WHITE,
  LISP_GC_GREY,
  LISP_GC_BLACK,
  LISP_GC_WHITE_ROOT,
  LISP_GC_GREY_ROOT,
  LISP_GC_BLACK_ROOT 
} lisp_gc_color_t;

typedef struct lisp_gc_collectible_list_t
{
  lisp_dl_list_t objects;
  lisp_gc_color_t color;
} lisp_gc_collectible_list_t;

typedef struct lisp_collectible_object_t
{
  lisp_gc_collectible_list_t * lst;
} lisp_collectible_object_t;

typedef struct lisp_gc_t
{
  void ** cons_pages;
  void * current_cons_page;
  lisp_dl_list_t recycled_conses;
  size_t cons_page_size;
  size_t num_cons_pages;
  size_t cons_pos;


  lisp_gc_collectible_list_t * white_list;
  lisp_gc_collectible_list_t * grey_list;
  lisp_gc_collectible_list_t * black_list;
  lisp_gc_collectible_list_t * white_root_list;
  lisp_gc_collectible_list_t * grey_root_list;
  lisp_gc_collectible_list_t * black_root_list;

} lisp_gc_t;

int lisp_init_gc(lisp_gc_t * gc);
int lisp_free_gc(lisp_gc_t * gc);

/**
 * Set the size of the cons pages.
 * Return LISP_OK or LISP_INVALID
 */
int lisp_gc_set_cons_page_size(lisp_gc_t * gc, size_t page_size);

struct lisp_cons_t * lisp_gc_alloc_cons(lisp_gc_t * gc);

void lisp_gc_free_cons(lisp_gc_t * gc, struct lisp_cons_t * cons);

#endif
