#include "gc_stat.h"
/* @todo implement general serializer for structures */

void lisp_init_gc_stat(lisp_gc_stat_t * stat)
{
  stat->num_root = 0;
  stat->num_bulk = 0;
  stat->num_reachable = 0;
  stat->num_allocated = 0;
  stat->num_recycled = 0;
  stat->num_void = 0;
  stat->num_disposed = 0;
  stat->num_cycles = 0;
  stat->num_leaves = 0;
  stat->num_edges = 0;
  stat->num_cons_pages = 0;
  stat->error_black_has_white_child = 0;
}

int lisp_gc_stat_eq(lisp_gc_stat_t * stat1,
                    lisp_gc_stat_t * stat2)
{
  return (stat1->num_root == stat2->num_root &&
          stat1->num_bulk == stat2->num_bulk &&
          stat1->num_reachable == stat2->num_reachable &&
          stat1->num_allocated == stat2->num_allocated &&
          stat1->num_recycled == stat2->num_recycled &&
          stat1->num_void == stat2->num_void &&
          stat1->num_disposed == stat2->num_disposed &&
          stat1->num_cycles == stat2->num_cycles &&
          stat1->num_leaves == stat2->num_leaves &&
          stat1->num_edges == stat2->num_edges &&
          stat1->num_cons_pages == stat2->num_cons_pages &&
          stat1->error_black_has_white_child ==
          stat2->error_black_has_white_child);
}


void lisp_gc_stat_print(FILE * fp, lisp_gc_stat_t * stat)
{
  fprintf(fp, "num_root:         % 5d\n", (int)stat->num_root);
  fprintf(fp, "num_bulk:         % 5d\n", (int)stat->num_bulk);
  fprintf(fp, "num_reachable:    % 5d\n", (int)stat->num_reachable);
  fprintf(fp, "num_allocated:    % 5d\n", (int)stat->num_allocated);
  fprintf(fp, "num_recycled:     % 5d\n", (int)stat->num_recycled);
  fprintf(fp, "num_void:         % 5d\n", (int)stat->num_void);
  fprintf(fp, "num_disposed:     % 5d\n", (int)stat->num_disposed);
  fprintf(fp, "num_cycles:       % 5d\n", (int)stat->num_cycles);
  fprintf(fp, "num_leaves:       % 5d\n", (int)stat->num_leaves);
  fprintf(fp, "num_edges:        % 5d\n", (int)stat->num_edges);
  fprintf(fp, "num_cons_pages:   % 5d\n", (int)stat->num_cons_pages);
  if(stat->error_black_has_white_child)
  {
    fprintf(fp, "error_black_has_white_child: true\n");
  }
  else
  {
    fprintf(fp, "error_black_has_white_child: false\n");
  }
}

void lisp_gc_stat_print2(FILE * fp,
                         lisp_gc_stat_t * stat1,
                         lisp_gc_stat_t * stat2)
{
  fprintf(fp, "num_root:         % 5d % 5d\n", (int)stat1->num_root,       (int)stat2->num_root);
  fprintf(fp, "num_bulk:         % 5d % 5d\n", (int)stat1->num_bulk,       (int)stat2->num_bulk);
  fprintf(fp, "num_reachable:    % 5d % 5d\n", (int)stat1->num_reachable,  (int)stat2->num_reachable);
  fprintf(fp, "num_allocated:    % 5d % 5d\n", (int)stat1->num_allocated,  (int)stat2->num_allocated);
  fprintf(fp, "num_recycled:     % 5d % 5d\n", (int)stat1->num_recycled,   (int)stat2->num_recycled);
  fprintf(fp, "num_void:         % 5d % 5d\n", (int)stat1->num_void,       (int)stat2->num_void);
  fprintf(fp, "num_disposed:     % 5d % 5d\n", (int)stat1->num_disposed,   (int)stat2->num_disposed);
  fprintf(fp, "num_cycles:       % 5d % 5d\n", (int)stat1->num_cycles,     (int)stat2->num_cycles);
  fprintf(fp, "num_leaves:       % 5d % 5d\n", (int)stat1->num_leaves,     (int)stat2->num_leaves);
  fprintf(fp, "num_edges:        % 5d % 5d\n", (int)stat1->num_edges,      (int)stat2->num_edges);
  fprintf(fp, "num_cons_pages:   % 5d % 5d\n", (int)stat1->num_cons_pages, (int)stat2->num_cons_pages);
  fprintf(fp, "error_black_has_white_child: %s %s\n",
          (stat1->error_black_has_white_child ? "true": "false"),
          (stat2->error_black_has_white_child ? "true": "false"));
}
