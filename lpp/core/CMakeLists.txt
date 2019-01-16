add_library( Core
  types/array.cpp
  types/forms/define.cpp
  gc/cons_pages.cpp
  gc/garbage_collector.cpp
  gc/symbol_container.cpp
  compiler/jit.cpp
  compiler/cons_pattern.cpp
  cell.cpp
  object.cpp
  default_env.cpp
  vm.cpp )
