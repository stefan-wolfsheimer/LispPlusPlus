#ifndef __LISP_GC_H__
#define __LISP_GC_H__
#include "gc_color_map.h"
#include <lisp/util/dl_list.h>
#include <stddef.h>
#include <stdio.h>
#include <stddef.h>
#include "typedefs.h"


/* dump modes for garbage collector dump function */
#define LISP_GC_DUMP_SILENT 0
#define LISP_GC_DUMP_HUMAN 1

struct lisp_gc_iterator_t;
struct lisp_gc_stat_t;

typedef struct lisp_gc_t
{
  /* active conses */
  lisp_gc_color_map_t cons_color_map;

  /* active objects */
  lisp_gc_color_map_t object_color_map;

  /* cons memory */
  void ** cons_pages;
  void * current_cons_page;
  lisp_dl_list_t recycled_conses;
  size_t cons_page_size;
  size_t num_cons_pages;
  size_t cons_pos;

  /* number of cycles */
  size_t num_cycles;
  size_t num_steps;
} lisp_gc_t;


/*****************************************************************************
 lisp_gc_t constructor / destructor
 ****************************************************************************/
/**
 * Initialize garbage collector object
 */
int lisp_init_gc(lisp_gc_t * gc);

/**
 * Free garbage collector object allocated with lisp_init_gc
 */
int lisp_free_gc(lisp_gc_t * gc);

/**
 * Defines the number of GC steps per allocation.
 * @return old value
 */
size_t lisp_gc_set_steps(lisp_gc_t * gc, size_t n);

/*****************************************************************************
 lisp_cons_t memory management functions
 ****************************************************************************/
/**
 * Set the size of the cons pages.
 *
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
 * The cons is added to the white (root) list.
 *
 * @return pointer to lisp_cons_t structure
 */
struct lisp_cons_t * lisp_gc_alloc_root_cons(lisp_gc_t * gc);

/**
 * Allocate a object
 * The object is added to the white list.
 *
 * @return pointer to object of size
 */
void * lisp_gc_alloc_object(lisp_gc_t * gc, lisp_type_id_t tid, size_t size);

/**
 * Allocate a root object
 * The object is added to the white (root) list.
 *
 * @return pointer to object of size
 */
void * lisp_gc_alloc_root_object(lisp_gc_t * gc, lisp_type_id_t tid, size_t size);

/**
 * free lisp_cons_t object allocated lisp_gc_alloc_cons.
 *
 * @param gc garbage collector object
 * @param cons object to be freed.
 */
void lisp_gc_free_cons(lisp_gc_t * gc, struct lisp_cons_t * cons);

/*****************************************************************************
 Garbage collector operation
 ****************************************************************************/
/**
 * @todo implement function
 */
void lisp_gc_step(lisp_gc_t * gc);
/**
 * @todo implement function
 */
short int lisp_gc_cons_step(lisp_gc_t * gc);
/**
 * @todo implement function
 */
short int lisp_gc_object_step(lisp_gc_t * gc);

/****************************************************************************
 lisp_cons_t properties and GC statistics
 ****************************************************************************/
/**
 * @todo move to cons module
 * Get the color of the cons in the 3 generation garbage collector.
 */
lisp_gc_color_t lisp_cons_get_color(const struct lisp_cons_t * cons);

/**
 * @todo move to cons module
 * Return true value if cons is in the root set
 */
short int lisp_cons_is_root(const struct lisp_cons_t * cons);

/**
 * Get number of allocated active conses.
 */
size_t lisp_gc_num_conses(lisp_gc_t * gc);

/**
 * Get total number of allocated conses.
 * Currently active plus recycled.
 */
size_t lisp_gc_num_allocated_conses(lisp_gc_t * gc);

/**
 * Get number of recycled conses
 */
size_t lisp_gc_num_recycled_conses(lisp_gc_t * gc);

/****************************************************************************
 lisp_gc_t consistency checks, dump and statistics
 ****************************************************************************/
int lisp_gc_check(lisp_gc_t * gc);

void lisp_gc_dump(FILE * fp, lisp_gc_t * gc, int mode);

void lisp_gc_get_stats(lisp_gc_t * gc,
                       struct lisp_gc_stat_t * stat);

#endif
