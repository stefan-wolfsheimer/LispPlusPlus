add_library( Core
  types/array.cpp
  gc/cons_pages.cpp
  gc/garbage_collector.cpp
  gc/symbol_container.cpp
  compiler/jit.cpp
  cell.cpp
  object.cpp
  default_env.cpp
  vm.cpp )
