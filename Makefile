CXX = g++
CXXFLAGS = -g -Wall -Iinclude

TEST_EXES = build/list_test

build/%.o : src/%.cpp
	$(CXX) $(CXXFLAGS) -c src/$*.cpp -o build/$*.o

build/%.o : tests/%.cpp
	$(CXX) $(CXXFLAGS) -Itests -c tests/$*.cpp -o build/$*.o

build/list_test : build/list_test.o build/tctest.o
	$(CXX) -o $@ $+

clean :
	rm -f build/*.o $(TEST_EXES)
