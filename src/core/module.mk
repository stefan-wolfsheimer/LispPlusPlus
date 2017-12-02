CORE_SRC=	src/core/lisp_type_id.cpp\
		src/core/lisp_object.cpp \
		src/core/lisp_cons_allocator.cpp \
		src/core/lisp_cons.cpp

SRC+=$(CORE_SRC)
SRC_TEST+=$(CORE_SRC)

SRC_TEST+=	src/core/test_object.cpp\
		src/core/test_cons_allocator.cpp\
		src/core/test_cons.cpp

