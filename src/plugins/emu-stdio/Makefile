CXX = g++
CXXFLAGS = -g -O3 -std=c++17 -shared -fPIC
LDFLAGS = -lncurses

INCLUDE = ../../include
SOURCES = main.cpp emu-stdin.cpp emu-stdout.cpp

emu-stdio.so: main.cpp emu-stdin.cpp emu-stdout.cpp
	$(CXX) -I$(INCLUDE) $(CXXFLAGS) -o ../$@ $^ $(LDFLAGS)
