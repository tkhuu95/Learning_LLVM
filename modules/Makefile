CXX=g++ -g

LINK_COMP=core executionengine mcjit support nativecodegen
LLVM=llvm-config --cxxflags --ldflags --system-libs --libs $(LINK_COMP)

add: add.cpp
	$(CXX) add.cpp `$(LLVM)` -o $@

displacement: displacement.cpp
	$(CXX) displacement.cpp `$(LLVM)` -o $@

clean:
	rm -f add displacement
