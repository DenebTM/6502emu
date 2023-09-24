CXX = g++
CXXFLAGS = -g -O3 -std=c++17 -shared -fPIC
LDFLAGS =

INCLUDE = ../../include
SOURCES = main.cpp pia1.cpp

pia1.so: $(SOURCES)
	$(CXX) -I$(INCLUDE) $(CXXFLAGS) -o ../$@ $^ $(LDFLAGS)
