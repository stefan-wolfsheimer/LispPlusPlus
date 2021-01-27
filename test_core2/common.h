#ifndef __COMMON_H__
#define __COMMON_H__
#include <lisp/core/vm.h>
#include <lisp/core/gc_stat.h>

#define LISP_GC_DUMP_TEST LISP_GC_DUMP_HUMAN
/* LISP_GC_DUMP_SILENT 0
   LISP_GC_DUMP_HUMAN 1
*/


#define ASSERT_LISP_OK(__TST__, __EXPR__) \
 ASSERT_EQ_I(__TST__, __EXPR__, LISP_OK);


#define ASSERT_LISP_CHECK_GC(__TST__, __VM__)                           \
  if(!CHECK_EQ_I((__TST__), lisp_vm_gc_check((__VM__)), LISP_OK))       \
  {                                                                     \
    lisp_vm_gc_dump(stdout, (__VM__), LISP_GC_DUMP_TEST);               \
    return ;                                                            \
  }

#define ASSERT_LISP_CHECK_GC_STATS(__TST__, __VM__, __REQ__)            \
{                                                                       \
   lisp_gc_stat_t _stat_;                                               \
   int _stat_ok_ = 1;                                                   \
   lisp_vm_gc_get_stats((__VM__), &_stat_);                             \
   if(!CHECK((__TST__), lisp_gc_stat_eq(&_stat_, (__REQ__))))           \
   {                                                                    \
     lisp_gc_stat_print2(stdout, &_stat_, (__REQ__));                   \
     _stat_ok_ = 0;                                                     \
   }                                                                    \
   if(!CHECK_EQ_I((__TST__), lisp_vm_gc_check((__VM__)), LISP_OK))      \
   {                                                                    \
     lisp_vm_gc_dump(stdout, (__VM__), LISP_GC_DUMP_TEST);              \
     _stat_ok_ = 0;                                                     \
   }                                                                    \
   if(!_stat_ok_)                                                       \
   {                                                                    \
     return;                                                            \
   }                                                                    \
 }

#endif
