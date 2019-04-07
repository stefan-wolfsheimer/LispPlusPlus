add_library( Core
  types/reference.cpp
  types/symbol.cpp
  types/function.cpp
  types/form.cpp
  types/forms/cons_of.cpp
  types/forms/list_of.cpp
  types/forms/choice_of.cpp
  types/forms/symbol_eq.cpp
  memory/cons_pages.cpp
  memory/allocator.cpp
  cell.cpp
  object.cpp
  exception.cpp
  util.cpp
  vm.cpp )
