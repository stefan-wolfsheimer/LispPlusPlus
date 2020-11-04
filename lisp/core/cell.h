#ifndef __LISP_CELL_H__
#define __LISP_CELL_H__

typedef unsigned short int lisp_type_id_t;

struct lisp_cons_t;

typedef struct lisp_cell_t
{
  lisp_type_id_t type_id;
  union
  {
    struct lisp_cons_t * cons;
  } data;
} lisp_cell_t;

extern lisp_cell_t lisp_nil;

int lisp_is_atomic(lisp_cell_t * cell);
int lisp_is_root_cell(lisp_cell_t * cell);

#endif
