#include "complex_object.h"

inline static lisp_dl_item_t *
_lisp_complex_object_as_dl_item(const lisp_complex_object_t * obj)
{
  return (lisp_dl_item_t*) (((lisp_dl_item_t*)obj) - 1);
}


void lisp_complex_object_grey(lisp_complex_object_t * obj)
{
    if(obj->gc_list->grey_elements != NULL)
    {
      lisp_dl_list_remove(&obj->gc_list->objects,
                          _lisp_complex_object_as_dl_item(obj));
      obj->gc_list = obj->gc_list->grey_elements;
      lisp_dl_list_append(&obj->gc_list->objects,
                          _lisp_complex_object_as_dl_item(obj));
    }
}
