CXX = g++
CXXFLAGS = -std=c++23 -shared -fPIC
LDFLAGS = -lncurses

emu-stdio.so: main.cpp emu-stdin.cpp emu-stdout.cpp
	$(CXX) $(CXXFLAGS) -o ../$@ $^ $(LDFLAGS)
