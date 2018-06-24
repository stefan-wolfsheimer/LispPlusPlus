add_library( Core
  types/lisp_symbol.cpp
  types/lisp_function.cpp
  types/array.cpp
  gc/cons_pages.cpp
  gc/garbage_collector.cpp
  gc/collectible_graph.cpp
  cell.cpp
  object.cpp
  symbol_factory.cpp
  vm.cpp
  compile.cpp )
