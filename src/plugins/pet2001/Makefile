CXX = g++
CXXFLAGS = -g -O3 -std=c++17 -shared -fPIC
LDFLAGS =

INCLUDE = ../../include
SOURCES = main.cpp chardev.cpp pia1.cpp

pet2001.so: $(SOURCES)
	$(CXX) -I$(INCLUDE) $(CXXFLAGS) -o ../$@ $^ $(LDFLAGS) `sdl2-config --cflags --libs`
