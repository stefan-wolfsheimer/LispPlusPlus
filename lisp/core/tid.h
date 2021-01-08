#ifndef __LIPS_TID_H__
#define __LIPS_TID_H__

#define LISP_NUM_TYPES 0x100

/**
 * storage classes
 * 00 atom
 * 01 managed object: requires constructor and destructor
 *                    but does not contain sub cells.
 *                    copy on write.
 * 10 reference:      immuatble managed object.
 *                    copied reference
 * 11 complex object: object that can contain sub cells
 *                    e.g. array or cons
 * xx 000 000
 * 0b00 << 6: 0x00
 * 0b01 << 6: 0x40
 * 0b10 << 6: 0x80
 * 0b11 << 6: 0xc0
 */
#define LISP_STORAGE_ATOM         0x00
#define LISP_STORAGE_OBJECT       0x40
#define LISP_STORAGE_REFERENCE    0x80
#define LISP_STORAGE_COMPLEX      0xc0

#define LISP_TID_NIL              (0x00 + 0)
#define LISP_TID_CONS             (0xc0 + 0)
#define LISP_TID_ARRAY            (0xc0 + 1)

#define LISP_IS_STORAGE_ATOM_TID(TID)           \
  (((TID) & 0xc0) == LISP_STORAGE_ATOM)

#define LISP_IS_STORAGE_OBJECT_TID(TID)         \
  (((TID) & 0xc0) == LISP_STORAGE_OBJECT)

#define LISP_IS_STORAGE_REFERENCE_TID(TID)      \
  (((TID) & 0xc0) == LISP_STORAGE_REFERENCE)

#define LISP_IS_STORAGE_COMPLEX_TID(TID)        \
  (((TID) & 0xc0) == LISP_STORAGE_COMPLEX)

#define LISP_IS_CONS_TID(TID) ((TID) == LISP_TID_CONS)
#define LISP_IS_NIL_TID(TID)  ((TID) == LISP_TID_NIL)


struct lisp_cell_t;
struct lisp_cell_iterator_t;

typedef struct lisp_type_t
{
  /**
   * This function is called before the
   * memory is freed.
   */
  int (*lisp_destructor_ptr)(void * ptr);
  int (*lisp_cell_first_child_ptr)(struct lisp_cell_iterator_t * itr);
  int (*lisp_cell_child_iterator_is_valid_ptr)(struct lisp_cell_iterator_t * itr);
  int (*lisp_cell_next_child_ptr)(struct lisp_cell_iterator_t * itr);
} lisp_type_t;

/**
 * Ensure that static types are initialized.
 */
int lisp_load_static_types();

extern lisp_type_t lisp_static_types[LISP_NUM_TYPES];

#endif
