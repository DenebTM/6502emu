CXX = g++
CXXFLAGS = -g -O3 -std=c++17 -shared -fPIC
LDFLAGS = -lncurses

emu-stdio.so: main.cpp emu-stdin.cpp emu-stdout.cpp
	$(CXX) $(CXXFLAGS) -o ../$@ $^ $(LDFLAGS)
