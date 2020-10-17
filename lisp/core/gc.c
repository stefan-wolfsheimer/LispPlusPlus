#include "gc.h"
#include "error.h"
#include <lisp/util/xmalloc.h>
#include <lisp/core/cons.h>

#define DEFAULT_PAGE_SIZE 1024

static lisp_gc_collectible_list_t * _new_gc_collectible_list(lisp_gc_color_t c)
{
  lisp_gc_collectible_list_t * lst = MALLOC(sizeof(lisp_gc_collectible_list_t));
  if(lst)
  {
    lisp_init_dl_list(&lst->objects);
    lst->color = c;
  }
  return lst;
}

int lisp_init_gc(lisp_gc_t * gc)
{
  gc->white_list = _new_gc_collectible_list(LISP_GC_WHITE);
  gc->grey_list = _new_gc_collectible_list(LISP_GC_GREY);
  gc->black_list = _new_gc_collectible_list(LISP_GC_BLACK);
  gc->white_root_list = _new_gc_collectible_list(LISP_GC_WHITE_ROOT);
  gc->grey_root_list = _new_gc_collectible_list(LISP_GC_GREY_ROOT);
  gc->black_root_list = _new_gc_collectible_list(LISP_GC_BLACK_ROOT);
  if(!gc->white_list ||
     !gc->grey_list ||
     !gc->black_list ||
     !gc->white_root_list ||
     !gc->grey_root_list ||
     !gc->black_root_list)
  {
    return LISP_BAD_ALLOC;
  }

  gc->cons_pages = NULL;
  lisp_init_dl_list(&gc->recycled_conses);
  gc->num_cons_pages = 0;
  gc->cons_pos = DEFAULT_PAGE_SIZE;
  gc->cons_page_size = DEFAULT_PAGE_SIZE;
  return LISP_OK;
}

int lisp_free_gc(lisp_gc_t * gc)
{
  FREE(gc->white_list);
  FREE(gc->grey_list);
  FREE(gc->black_list);
  FREE(gc->white_root_list);
  FREE(gc->grey_root_list);
  FREE(gc->black_root_list);
  return LISP_OK;
}

int lisp_gc_set_cons_page_size(lisp_gc_t * gc, size_t page_size)
{
  if(gc->num_cons_pages)
  {
    return LISP_INVALID;
  }
  else
  {
    gc->cons_pos = page_size;
    gc->cons_page_size = page_size;
    return LISP_OK;
  }
}

lisp_cons_t * lisp_gc_alloc_cons(lisp_gc_t * gc)
{
  if(!lisp_dl_list_empty(&gc->recycled_conses))
  {
    lisp_dl_item_t * ret = gc->recycled_conses.first;
    ret++;
    lisp_dl_list_remove_first(&gc->recycled_conses);
    return (lisp_cons_t*)ret;
  }
  if(gc->cons_pos >= gc->cons_page_size)
  {
    gc->current_cons_page = MALLOC((sizeof(lisp_dl_item_t) +
                                    sizeof(lisp_cons_t)) * gc->cons_page_size);
    gc->cons_pages = REALLOC(gc->cons_pages,
                             sizeof(void*) * (gc->num_cons_pages + 1));
    if(!gc->cons_pages)
    {
      return NULL;
    }
    gc->cons_pages[gc->num_cons_pages++] = gc->current_cons_page;
    gc->cons_pos = 0;
  }
  return (lisp_cons_t*)(
                        (char*) gc->current_cons_page +
                        ((sizeof(lisp_dl_item_t) +
                          sizeof(lisp_cons_t)) * gc->cons_pos++) +
                        sizeof(lisp_dl_item_t));
}

void lisp_gc_free_cons(lisp_gc_t * gc, lisp_cons_t * cons)
{
  lisp_dl_list_append(&gc->recycled_conses,
                      ((lisp_dl_item_t*)cons) - 1);
}
