#include <lisp/core/cons.h>
#include <lisp/util/xmalloc.h>

/* @todo move to header */
#define LISP_OK 0
#define INVALID_ALLOC 1

#define DEFAULT_PAGE_SIZE 1024

int lisp_init_cons_pages(lisp_cons_pages_t * pages)
{
  pages->pages = NULL;
  lisp_init_dl_list(&pages->recycled);
  pages->num_pages = 0;
  pages->pos = DEFAULT_PAGE_SIZE;
  pages->page_size = DEFAULT_PAGE_SIZE;
  return LISP_OK;
}

int lisp_cons_pages_set_size(lisp_cons_pages_t * pages, size_t page_size)
{
  if(pages->num_pages)
  {
    return INVALID_ALLOC;
  }
  else
  {
    pages->pos = page_size;
    pages->page_size = page_size;
    return LISP_OK;
  }
}

lisp_cons_t * lisp_cons_alloc(lisp_cons_pages_t * pages)
{
  if(!lisp_dl_list_empty(&pages->recycled))
  {
    //@todo remove
    // return
  }
  if(pages->pos >= pages->page_size)
  {
    pages->current_page = MALLOC((sizeof(lisp_dl_item_t) +
                                  sizeof(lisp_cons_t)) * pages->page_size);
    pages->pages = REALLOC(pages->pages,
                           sizeof(lisp_dl_item_t*) * (pages->num_pages + 1));
    if(!pages->pages)
    {
      return NULL;
    }
    pages->pages[pages->num_pages++] = pages->current_page;
    pages->pos = 0;
  }
  return (lisp_cons_t*)(
                        (char*) pages->current_page +
                        ((sizeof(lisp_dl_item_t) +
                          sizeof(lisp_cons_t)) * pages->pos++) +
                        sizeof(lisp_dl_item_t));
}

void lisp_cons_free(lisp_cons_pages_t * pages, lisp_cons_t * cons)
{
  lisp_dl_list_append(&pages->recycled,
                      ((lisp_dl_item_t*)cons) - 1);
}
