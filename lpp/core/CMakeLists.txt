add_library( Core
  types/array.cpp
  types/reference.cpp
  types/symbol.cpp
  types/form.cpp
  types/forms/cons_of.cpp
  types/forms/list_of.cpp
  types/forms/choice_of.cpp
  gc/cons_pages.cpp
  gc/garbage_collector.cpp
  gc/symbol_container.cpp
  compiler/jit.cpp
  compiler/forms/define.cpp
  compiler/forms/lambda.cpp
  cell.cpp
  object.cpp
  exception.cpp
  util.cpp
  default_env.cpp
  vm.cpp )
