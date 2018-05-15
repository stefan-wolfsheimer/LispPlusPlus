#include "core/gc/garbage_collector.h"

using GarbageCollector = Lisp::GarbageCollector;

GarbageCollector::GarbageCollector(std::size_t consPageSize,
                                   unsigned short _garbageSteps,
                                   unsigned short _recycleSteps)
  : consPages(consPageSize),
    garbageSteps(_garbageSteps),
    recycleSteps(_recycleSteps),
    backGarbageSteps(_garbageSteps),
    backRecycleSteps(_recycleSteps)
{
}
