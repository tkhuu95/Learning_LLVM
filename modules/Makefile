CXX=g++ -g

LINK_COMP=core executionengine mcjit support nativecodegen
LLVM=llvm-config --cxxflags --ldflags --system-libs --libs $(LINK_COMP)

add:
	$(CXX) add.cpp `$(LLVM)` -o $@

clean:
	rm -f add
