MODULES= core
CPP=g++
CPP_TEST_FLAGS=-Wall -std=c++11 -g -Isrc -Itest
CPP_FLAGS=-Wall -std=c++11 -Isrc
DEPFLAGS= -MT $@ -MMD -MP -MF release/obj/$*.td
TEST_DEPFLAGS= -MT $@ -MMD -MP -MF test/obj/$*.td
BINDIR=bin

SRC=
SRC_TEST=test/runtests.cpp

all: 	release/liblisp.a\
	bin/runtests

include $(patsubst %, src/%/module.mk, $(MODULES) )

OBJ= $(patsubst src/%.cpp, release/obj/%.o, $(SRC) ) 
OBJ_TEST= $(patsubst src/%.cpp, test/obj/%.o, $(SRC_TEST) )

release/liblisp.a: $(OBJ)
	ar -cvr release/liblisp.a $(OBJ)

release/obj/%.o : src/%.cpp
	mkdir -p $(@D)
	$(CPP) ${CPP_FLAGS} ${DEPFLAGS} -c $< -o $@
	mv release/obj/$*.td release/obj/$*.d 

bin/runtests: $(OBJ_TEST)
	mkdir -p $(@D)
	${CPP} ${CPP_TEST_FLAGS} ${OBJ_TEST} -o bin/runtests

# bin/sim_gc: src/simulation/sim_gc.cpp release/liblisp.a
#	mkdir -p $(@D)
#	${CPP} ${CPP_FLAGS} ${DEPFLAGS} -Lrelease \
#	src/simulation/sim_gc.cpp -llisp -o bin/sim_gc

test/obj/%.o : src/%.cpp
	mkdir -p $(@D)
	${CPP} ${CPP_TEST_FLAGS} ${TEST_DEPFLAGS} -c $< -o $@
	mv test/obj/$*.td test/obj/$*.d 

.d: ;
.PRECIOUS: %.d

-include $(patsubst src/%.cpp, release/obj/%.d, ${SRC})
-include $(patsubst src/%.cpp, test/obj/%.d, ${SRC_TEST})

clean:
	rm -f test/obj/*.o
	rm -f test/obj/*.d
	rm -f test/obj/*/*.o
	rm -f test/obj/*/*.d
	rm -f release/obj/*.o
	rm -f release/obj/*.d
	rm -f release/obj/*/*.o
	rm -f release/obj/*/*.d
