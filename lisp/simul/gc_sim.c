#include "gc_sim.h"
#include <lisp/util/xmalloc.h>
#include <lisp/util/hash_table.h>
#include <lisp/core/error.h>
#include <lisp/core/cons.h>
#include <lisp/core/array.h>
#include <lisp/core/gc_iterator.h>
#include <lisp/core/cell_iterator.h>
#include <lisp/core/cell_hash_table.h>
#include <lisp/core/tid.h>


/*****************************************************************************
 * Auxillary function
 *****************************************************************************/
inline static void _lisp_sim_init_reachable(lisp_gc_sim_t * sim,
                                            hash_table_t * reachable)
{
  lisp_gc_reachable_iterator_t ritr;
  int inserted;
  lisp_init_gc_reachable_iterator(&ritr);
  for(lisp_gc_reachable_first(&sim->vm, &ritr);
      lisp_gc_reachable_iterator_is_valid(&ritr);
      lisp_gc_reachable_next(&sim->vm, &ritr))
  {
    inserted = 0;
    lisp_cell_hash_table_find_or_insert(reachable,
                                        &ritr.cell,
                                        &inserted);
    assert(inserted);
  }
  assert(HASH_TABLE_SIZE(&ritr.root) == HASH_TABLE_SIZE(reachable));
  lisp_free_gc_reachable_iterator(&ritr);
}

inline static size_t  _lisp_sim_init_nodes(hash_table_t * reachable,
                                           lisp_gc_sim_node_t ** nodes)
{
  hash_table_entry_t * entry;
  size_t num_nodes;
  size_t i;
  lisp_cell_t * cell;
  num_nodes = HASH_TABLE_SIZE(reachable);
  *nodes = ((num_nodes == 0) ? NULL :
            MALLOC(num_nodes * sizeof(lisp_gc_sim_node_t)));
  i = 0;
  for(entry = HASH_TABLE_FIRST(reachable);
      entry != NULL;
      entry = HASH_TABLE_NEXT(entry))
  {
    cell = HASH_TABLE_DATA(entry, lisp_cell_t);
    *((lisp_gc_sim_node_t **)(cell + 1)) = *nodes + i;
    (*nodes)[i].is_root = lisp_is_root_cell(cell);
    (*nodes)[i].node_index = i;
    (*nodes)[i].cell = *cell;
    i++;
  }
  return num_nodes;
}

inline static size_t _lisp_sim_num_edges(hash_table_t * reachable)
{
  size_t num_edges;
  hash_table_entry_t * entry;
  lisp_cell_t * cell;
  lisp_cell_iterator_t citr;
  num_edges = 0;
  for(entry = HASH_TABLE_FIRST(reachable);
      entry != NULL;
      entry = HASH_TABLE_NEXT(entry))
  {
    cell = HASH_TABLE_DATA(entry, lisp_cell_t);
    for(lisp_first_child(cell, &citr);
        lisp_cell_iterator_is_valid(&citr);
        lisp_cell_next_child(&citr))
    {
      if(LISP_STORAGE_ID(citr.child->type_id) == LISP_STORAGE_CONS ||
         LISP_STORAGE_ID(citr.child->type_id) == LISP_STORAGE_COMPLEX)
      {
        num_edges++;
      }
    }

  }
  return num_edges;
}

inline static size_t _lisp_sim_init_edges(hash_table_t * reachable,
                                          lisp_gc_sim_edge_t ** edges)
{
  hash_table_entry_t * entry;
  lisp_cell_t * cell;
  lisp_cell_t * child;
  lisp_cell_t * parent;
  lisp_cell_iterator_t citr;
  size_t num_edges;
  size_t j;
  num_edges = _lisp_sim_num_edges(reachable);
  if(num_edges)
  {
    *edges = MALLOC(num_edges * sizeof(lisp_gc_sim_edge_t));
    num_edges = 0;
    for(entry = HASH_TABLE_FIRST(reachable);
        entry != NULL;
        entry = HASH_TABLE_NEXT(entry))
    {
      cell = HASH_TABLE_DATA(entry, lisp_cell_t);
      parent = lisp_cell_hash_table_find(reachable, cell);
      j = 0;
      for(lisp_first_child(cell, &citr);
          lisp_cell_iterator_is_valid(&citr);
          lisp_cell_next_child(&citr))
      {
        if(LISP_STORAGE_ID(citr.child->type_id) == LISP_STORAGE_CONS ||
           LISP_STORAGE_ID(citr.child->type_id) == LISP_STORAGE_COMPLEX)
        {
          (*edges)[num_edges].index = j;
          if(parent)
          {
            (*edges)[num_edges].parent = *(lisp_gc_sim_node_t **)(parent + 1);
          }
          else
          {
            (*edges)[num_edges].parent = NULL;
          }
          child = lisp_cell_hash_table_find(reachable, citr.child);
          if(child)
          {
            (*edges)[num_edges].child = *(lisp_gc_sim_node_t **)(child + 1);
          }
          else
          {
            (*edges)[num_edges].child = NULL;
          }
          num_edges++;
        }
        j++;
      }
    }
  }
  else
  {
    *edges = NULL;
  }
  return num_edges;
}

inline static size_t _lisp_sim_num_leaves(hash_table_t * reachable)
{
  size_t num_leaves;
  hash_table_entry_t * entry;
  lisp_cell_t * cell;
  lisp_cell_iterator_t citr;
  num_leaves = 0;
  for(entry = HASH_TABLE_FIRST(reachable);
      entry != NULL;
      entry = HASH_TABLE_NEXT(entry))
  {
    cell = HASH_TABLE_DATA(entry, lisp_cell_t);
    for(lisp_first_child(cell, &citr);
        lisp_cell_iterator_is_valid(&citr);
        lisp_cell_next_child(&citr))
    {
      if(LISP_STORAGE_ID(citr.child->type_id) != LISP_STORAGE_CONS &&
         LISP_STORAGE_ID(citr.child->type_id) != LISP_STORAGE_COMPLEX)
      {
        num_leaves++;
      }
    }
  }
  return num_leaves;
}

inline static size_t _lisp_sim_init_leaves(hash_table_t * reachable,
                                           lisp_gc_sim_leaf_t ** leaves)
{
  hash_table_entry_t * entry;
  lisp_cell_t * parent;
  lisp_cell_t * cell;
  size_t j;
  size_t num_leaves;
  lisp_cell_iterator_t citr;
  num_leaves = _lisp_sim_num_leaves(reachable);
  if(num_leaves)
  {
    *leaves = MALLOC(num_leaves * sizeof(lisp_gc_sim_leaf_t));
    num_leaves = 0;
    for(entry = HASH_TABLE_FIRST(reachable);
        entry != NULL;
        entry = HASH_TABLE_NEXT(entry))
    {
      cell = HASH_TABLE_DATA(entry, lisp_cell_t);
      parent = lisp_cell_hash_table_find(reachable, cell);
      j = 0;
      for(lisp_first_child(cell, &citr);
          lisp_cell_iterator_is_valid(&citr);
          lisp_cell_next_child(&citr))
      {
        if(LISP_STORAGE_ID(citr.child->type_id) != LISP_STORAGE_CONS &&
           LISP_STORAGE_ID(citr.child->type_id) != LISP_STORAGE_COMPLEX)
        {
          if(parent)
          {
            (*leaves)[num_leaves].node = *((lisp_gc_sim_node_t **)(parent + 1));
          }
          else
          {
            (*leaves)[num_leaves].node = NULL;
          }
          (*leaves)[num_leaves].index = j;
          num_leaves++;
        }
        j++;
      } /* for each child */
    }
  }
  else
  {
    *leaves = 0;
  }
  return num_leaves;
}

inline static int _lisp_sim_refresh_graph(lisp_gc_sim_t * sim)
{
  int ret;
  lisp_free_cell_hash_table(&sim->reachable);
  ret = lisp_init_cell_hash_map(&sim->reachable, sizeof(lisp_gc_sim_node_t*));
  if(ret != LISP_OK)
  {
    return ret;
  }
  _lisp_sim_init_reachable(sim, &sim->reachable);
  if(sim->leaves)
  {
    FREE(sim->leaves);
    sim->leaves = NULL;
  }
  if(sim->edges)
  {
    FREE(sim->edges);
    sim->edges = NULL;
  }
  if(sim->nodes)
  {
    FREE(sim->nodes);
    sim->nodes = NULL;
  }
  sim->num_nodes = _lisp_sim_init_nodes(&sim->reachable, &sim->nodes);
  sim->num_edges = _lisp_sim_init_edges(&sim->reachable, &sim->edges);
  sim->num_leaves = _lisp_sim_init_leaves(&sim->reachable, &sim->leaves);
  return LISP_OK;
}

/*****************************************************************************
 * constructor / destructor
 *****************************************************************************/
int lisp_init_gc_sim(lisp_gc_sim_t * sim)
{
  int ret;
  sim->root = NULL;
  sim->num_root = 0;
  
  sim->nodes = NULL;
  sim->num_nodes = 0;

  sim->leaves = NULL;
  sim->num_leaves = 0;

  sim->edges = NULL;
  sim->num_edges = 0;

  ret = lisp_init_cell_hash_map(&sim->reachable,
                                sizeof(lisp_gc_sim_node_t*));
  if(ret != LISP_OK)
  {
    return ret;
  }
  return lisp_init_vm(&sim->vm);
}

int lisp_free_gc_sim(lisp_gc_sim_t * sim)
{
  size_t i;
  int ret;
  ret = LISP_OK;
  for(i = 0; i < sim->num_root; i++)
  {
    ret = lisp_unset(sim->root[i]);
    FREE(sim->root[i]);
    if(ret != LISP_OK)
    {
        break;
    }
  }
  if(ret != LISP_OK)
  {
    return ret;
  }
  if(sim->root)
  {
    FREE(sim->root);
  }
  if(sim->nodes)
  {
    FREE(sim->nodes);
  }
  if(sim->leaves)
  {
    FREE(sim->leaves);
  }
  if(sim->edges)
  {
    FREE(sim->edges);
  }
  lisp_free_cell_hash_table(&sim->reachable);
  return lisp_free_vm(&sim->vm);
}

lisp_gc_sim_node_t * lisp_sim_find_node(lisp_gc_sim_t * sim,
                                        const lisp_cell_t * cell)
{
  lisp_cell_t * parent;
  parent = lisp_cell_hash_table_find(&sim->reachable, cell);
  if(parent)
  {
    return *((lisp_gc_sim_node_t**)(parent + 1));
  }
  else
  {
    return NULL;
  }
}

lisp_cell_t *  lisp_sim_add_root(lisp_gc_sim_t * sim,
                                 size_t n_children)
{
  lisp_cell_t ** root;
  lisp_cell_t * cell;
  int ret;
  root = REALLOC(sim->root, (sim->num_root + 1) * sizeof(lisp_cell_t * ));
  if(root == NULL)
  {
    return NULL;
  }
  cell = MALLOC(sizeof(lisp_cell_t));
  root[sim->num_root] = cell;
  lisp_make_nil(cell);
  sim->num_root++;
  sim->root = root;
  if(n_children == 2)
  {
    ret = lisp_make_cons(&sim->vm, cell, NULL, NULL);
  }
  else
  {
    ret = lisp_make_array(&sim->vm, cell, n_children, NULL);
  }
  if(ret != LISP_OK)
  {
    return NULL;
  }
  ret = _lisp_sim_refresh_graph(sim);
  if(ret != LISP_OK)
  {
    return NULL;
  }
  return cell;
}

int lisp_sim_add_bulk(lisp_gc_sim_t * sim,
                      lisp_cell_t * parent,
                      size_t index,
                      lisp_cell_t  * child,
                      size_t n_children)
{
  int ret;
  lisp_cell_t cell;
  if(n_children == 2)
  {
    ret = lisp_make_temp_cons(&sim->vm,
                              &cell,
                              NULL,
                              NULL);
  }
  else
  {
    ret = lisp_make_temp_array(&sim->vm,
                               &cell,
                               n_children,
                               NULL);
  }
  if(ret != LISP_OK)
  {
    return ret;
  }

  if(lisp_is_cons(parent))
  {
    if(index == 0)
    {
      ret = lisp_cons_set_car(lisp_as_cons(parent), &cell);
      *child = lisp_as_cons(parent)->car;
    }
    else if(index == 1)
    {
      ret = lisp_cons_set_cdr(lisp_as_cons(parent), &cell);
      *child = lisp_as_cons(parent)->cdr;
    }
    else
    {
      ret = LISP_RANGE_ERROR;
    }
  }
  else if(lisp_is_array(parent))
  {
    if(index < lisp_array_size(lisp_as_array(parent)))
    {
      ret = lisp_array_set(lisp_as_array(parent), index, &cell);
      *child = lisp_as_array(parent)->data[index];
    }
    else
    {
      ret = LISP_RANGE_ERROR;
    }
  }
  else
  {
    ret = LISP_NOT_IMPLEMENTED;
  }
  if(ret != LISP_OK)
  {
    return ret;
  }
  ret = _lisp_sim_refresh_graph(sim);
  return ret;
}

int lisp_sim_add_edge(lisp_gc_sim_t * sim,
                      lisp_cell_t * parent,
                      size_t index,
                      lisp_cell_t  * child)
{
  if(lisp_is_cons(parent))
  {
    if(index == 0)
    {
      assert(lisp_is_nil(&lisp_as_cons(parent)->car));
      //@todo check return
      lisp_cons_set_car(lisp_as_cons(parent), child);
      return _lisp_sim_refresh_graph(sim);
    }
    else if(index == 1)
    {
      assert(lisp_is_nil(&lisp_as_cons(parent)->cdr));
      //@todo check return
      lisp_cons_set_cdr(lisp_as_cons(parent), child);
      return _lisp_sim_refresh_graph(sim);
    }
    else
    {
      return LISP_RANGE_ERROR;
    }
  }
  else if(lisp_is_array(parent))
  {
    if(index < lisp_array_size(lisp_as_array(parent)))
    {
      assert(lisp_is_nil(lisp_array_get(lisp_as_array(parent), index)));
      //@todo check return
      lisp_array_set(lisp_as_array(parent), index, child);
      return _lisp_sim_refresh_graph(sim);
    }
    else
    {
      return LISP_RANGE_ERROR;
    }
  }
  else
  {
    return LISP_NOT_IMPLEMENTED;
  }
}

int lisp_sim_remove_edge(lisp_gc_sim_t * sim,
                         lisp_cell_t * parent,
                         size_t index)
{
  int ret;
  if(lisp_is_cons(parent))
  {
    if(index == 0)
    {
      ret = lisp_cons_unset_car(lisp_as_cons(parent));
    }
    else if(index == 1)
    {
      ret = lisp_cons_unset_cdr(lisp_as_cons(parent));
    }
    else
    {
      ret = LISP_RANGE_ERROR;
    }
  }
  else if(lisp_is_array(parent))
  {
    if(index < lisp_array_size(lisp_as_array(parent)))
    {
      ret = lisp_array_unset(lisp_as_array(parent), index);
    }
    else
    {
      ret = LISP_RANGE_ERROR;
    }
  }
  else
  {
    ret = LISP_NOT_IMPLEMENTED;
  }
  if(ret != LISP_OK)
  {
    return ret;
  }
  return _lisp_sim_refresh_graph(sim);
}

int lisp_sim_unroot(lisp_gc_sim_t * sim, lisp_cell_t  * cell)
{
  int ret;
  size_t i;
  if(lisp_is_root_cell(cell))
  {
    ret = lisp_unset(cell);
    for(i = 0; i < sim->num_root; i++)
    {
      if(sim->root[i] == cell)
      {
        sim->root[i] = sim->root[sim->num_root - 1];
        sim->num_root--;
        FREE(cell);
        break;
      }
    }
    if(ret != LISP_OK)
    {
      return ret;
    }
    return _lisp_sim_refresh_graph(sim);
  }
  else
  {
    return LISP_INVALID;
  }
}
