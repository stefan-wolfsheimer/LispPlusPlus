add_library( Core
  types/lisp_cons.cpp
  types/lisp_symbol.cpp
  types/lisp_function.cpp
  gc/cons_pages.cpp
  gc/garbage_collector.cpp
  gc/collectible_graph.cpp
  lisp_cell.cpp
  lisp_object.cpp
  lisp_symbol_factory.cpp
  lisp_vm.cpp
  lisp_compile.cpp )
