CXX = g++
CXXFLAGS = -g -Wall -Iinclude -DDSLIB_CHECK_INTEGRITY

SRCS = ds_list.cpp ds_aatree.cpp ds_aatreeprint.cpp
OBJS = $(SRCS:%.cpp=build/%.o)

TEST_SRCS = tctest.cpp list_test.cpp aatree_test.cpp

TEST_EXES = build/list_test build/aatree_test

build/%.o : src/%.cpp
	$(CXX) $(CXXFLAGS) -c src/$*.cpp -o build/$*.o

build/%.o : tests/%.cpp
	$(CXX) $(CXXFLAGS) -Itests -c tests/$*.cpp -o build/$*.o

all : $(TEST_EXES)

build/list_test : build/list_test.o build/tctest.o $(OBJS)
	$(CXX) -o $@ $+

build/aatree_test : build/aatree_test.o build/tctest.o $(OBJS)
	$(CXX) -o $@ $+

clean :
	rm -f build/*.o $(TEST_EXES)

depend :
	$(CXX) $(CXXFLAGS) -M $(SRCS:%=src/%) $(TEST_SRCS:%=tests/%) \
		| ./scripts/fixdeps.rb \
		> depend.mak

depend.mak :
	touch $@

include depend.mak
