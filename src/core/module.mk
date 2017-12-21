CORE_SRC=	src/core/lisp_type_id.cpp\
		src/core/lisp_cell.cpp\
		src/core/lisp_object.cpp \
		src/core/lisp_cons_factory.cpp \
		src/core/lisp_cons.cpp \
		src/core/lisp_vm.cpp

SRC+=$(CORE_SRC)
SRC_TEST+=$(CORE_SRC)

SRC_TEST+=	src/core/test_object.cpp\
		src/core/test_cons_factory.cpp\
		src/core/test_cons.cpp

