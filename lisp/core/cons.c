/******************************************************************************
Copyright (c) 2021, Stefan Wolfsheimer

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.
******************************************************************************/
#include <assert.h>
#include <lisp/util/dl_list.h>
#include <lisp/util/xmalloc.h>
#include "vm.h"
#include "error.h"
#include "cons.h"
#include "complex_object.h"
#include "tid.h"
#include "cell_iterator.h"
#include "gc_color_map.h"

/*****************************************************************************
 constructor
 ****************************************************************************/
inline static lisp_dl_item_t * _lisp_cons_as_dl_item(const lisp_cons_t * cons)
{
  return (lisp_dl_item_t*) (((lisp_dl_item_t*)cons) - 1);
}

static lisp_cons_t * _lisp_gc_alloc_cons(lisp_vm_t * vm,
                                         lisp_gc_collectible_list_t * list,
                                         size_t ref_count)
{
  lisp_dl_item_t * item;
  if(!lisp_dl_list_empty(&vm->recycled_conses))
  {
    item = vm->recycled_conses.first;
    lisp_dl_list_remove_first(&vm->recycled_conses);
  }
  else
  {
    if(vm->cons_pos >= vm->cons_page_size)
    {
      vm->current_cons_page = MALLOC((sizeof(lisp_dl_item_t) +
                                      sizeof(lisp_cons_t)) * vm->cons_page_size);
      vm->cons_pages = REALLOC(vm->cons_pages,
                               sizeof(void*) * (vm->num_cons_pages + 1));
      if(!vm->cons_pages)
      {
        return NULL;
      }
      vm->cons_pages[vm->num_cons_pages++] = vm->current_cons_page;
      vm->cons_pos = 0;
    }
    item = (lisp_dl_item_t*)(
                             (char*) vm->current_cons_page +
                             ((
                               sizeof(lisp_dl_item_t) +
                               sizeof(lisp_cons_t)) *
                              vm->cons_pos++));
  }
  lisp_dl_list_append(&list->objects, item);
  lisp_cons_t * ret = (lisp_cons_t*)((
                                      (char*) item) +
                                     sizeof(lisp_dl_item_t));
  ret->ref_count = ref_count;
  ret->gc_list = list;
  ret->car.type_id = LISP_TID_NIL;
  ret->cdr.type_id = LISP_TID_NIL;
  return ret;
}

static inline void _lisp_cons_grey(lisp_cons_t * cons)
{
  if(cons->gc_list->grey_elements != NULL)
  {
    lisp_dl_list_remove(&cons->gc_list->objects,
                        _lisp_cons_as_dl_item(cons));
    cons->gc_list = cons->gc_list->grey_elements;
    lisp_dl_list_append(&cons->gc_list->objects,
                        _lisp_cons_as_dl_item(cons));
  }
}

void lisp_cons_grey(lisp_cons_t * cons)
{
  _lisp_cons_grey(cons);
}

/******************************************************************************
 * car / cdr modification
 ******************************************************************************/
static int _lisp_unset_cons_cell(lisp_cell_t * target)
{
  switch(LISP_STORAGE_ID(target->type_id))
  {
  case LISP_STORAGE_ATOM:
  case LISP_STORAGE_NULL:
    target->type_id = LISP_TID_NIL;
    return LISP_OK;
    break;
  case LISP_STORAGE_COW_OBJECT:
    return LISP_NOT_IMPLEMENTED;
    break;
  case LISP_STORAGE_OBJECT:
    return LISP_NOT_IMPLEMENTED;
    break;
  case LISP_STORAGE_CONS:
    target->type_id = LISP_TID_NIL;
    target->data.obj = NULL;
    return LISP_OK;
  case LISP_STORAGE_COMPLEX:
    target->type_id = LISP_TID_NIL;
    target->data.obj = NULL;
    return LISP_OK;
    break;
  default:
    return LISP_NOT_IMPLEMENTED;
  }
  return LISP_OK;
}

int lisp_cons_unset_car(lisp_cons_t * cons)
{
  return _lisp_unset_cons_cell(&cons->car);
}

int lisp_cons_unset_cdr(lisp_cons_t * cons)
{
  return _lisp_unset_cons_cell(&cons->cdr);
}

int lisp_cons_unset_car_cdr(lisp_cons_t * cons)
{
  int ret = _lisp_unset_cons_cell(&cons->car);
  if(ret != LISP_OK)
  {
    return ret;
  }
  return _lisp_unset_cons_cell(&cons->cdr);
}

int lisp_cons_set_car(lisp_cons_t * cons,
                      lisp_cell_t * car)
{
  return LISP_NOT_IMPLEMENTED;
}

int lisp_cons_set_cdr(lisp_cons_t * cons,
                      lisp_cell_t * cdr)
{
  return LISP_NOT_IMPLEMENTED;
}



static int _lisp_make_cons_cell(lisp_vm_t * vm,
                                lisp_cell_t * target,
                                const lisp_cell_t * source)
{
  /*@todo unit test coverage */
  if(source)
  {
    switch(LISP_STORAGE_ID(source->type_id))
    {
    case LISP_STORAGE_ATOM:
      target->data = source->data;
    case LISP_STORAGE_NULL:
      target->type_id = source->type_id;
      return LISP_OK;
      break;
    case LISP_STORAGE_COW_OBJECT:
      return LISP_NOT_IMPLEMENTED;
      break;
    case LISP_STORAGE_OBJECT:
      return LISP_NOT_IMPLEMENTED;
      break;
    case LISP_STORAGE_CONS:
      /* ensure that child is not white. */
      _lisp_cons_grey((lisp_cons_t*)source->data.obj);
      target->type_id = source->type_id;
      target->data = source->data;
      return LISP_OK;
    case LISP_STORAGE_COMPLEX:
      /* ensure that child is not white */
      lisp_complex_object_grey((lisp_complex_object_t*)source->data.obj);
      target->type_id = source->type_id;
      target->data = source->data;
      return LISP_OK;
      break;
    }
    /*@todo implement other types */
    return LISP_NOT_IMPLEMENTED;
  }
  else
  {
    target->type_id = LISP_TID_NIL;
  }
  return LISP_OK;
}

int lisp_make_cons(lisp_vm_t * vm,
                   lisp_cell_t * cell,
                   const lisp_cell_t * car,
                   const lisp_cell_t * cdr)
{
  int ret;
  cell->type_id = LISP_TID_CONS;
  cell->data.obj = _lisp_gc_alloc_cons(vm,
                                       vm->cons_color_map.white_root,
                                       1u);

  if(!cell->data.obj)
  {
    return LISP_BAD_ALLOC;
  }
  ret = _lisp_make_cons_cell(vm,
                             &((lisp_cons_t*)cell->data.obj)->car,
                             car);
  if(!ret)
  {
    return _lisp_make_cons_cell(vm,
                                &((lisp_cons_t*)cell->data.obj)->cdr,
                                cdr);
  }
  else
  {
    return ret;
  }
}


static int _cons_first_child(lisp_cell_iterator_t * itr)
{
  assert(lisp_is_cons(itr->parent));
  itr->child = &lisp_as_cons(itr->parent)->car;
  return LISP_OK;
}

static int _cons_iterator_is_valid(lisp_cell_iterator_t * itr)
{
  return (itr->child != NULL);
}

static int _cons_next_child(struct lisp_cell_iterator_t * itr)
{
  assert(lisp_is_cons(itr->parent));
  if(itr->child == &lisp_as_cons(itr->parent)->car)
  {
    itr->child = &lisp_as_cons(itr->parent)->cdr;
  }
  else
  {
    itr->child = NULL;
  }
  return (itr->child != NULL);
}

int lisp_init_cons_type(struct lisp_type_t * t)
{
  t->lisp_cell_first_child_ptr = _cons_first_child;
  t->lisp_cell_child_iterator_is_valid_ptr = _cons_iterator_is_valid;
  t->lisp_cell_next_child_ptr = _cons_next_child;
  return LISP_OK;
}
