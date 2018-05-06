#include "core/gc/garbage_collector.h"

using GarbageCollector = Lisp::GarbageCollector;

GarbageCollector::GarbageCollector(std::size_t consPageSize)
  : consPages(consPageSize)
{
}
