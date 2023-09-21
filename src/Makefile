CXX = g++
CXXFLAGS = -g -funsigned-char -std=c++17
LDFLAGS = -lm -ldl -lreadline

INCLUDE = include
SOURCES = main.cpp mem.cpp cpu.cpp

.PHONY: clean rebuild plugins

all: 6502 plugins

6502: $(SOURCES)
	$(CXX) -I$(INCLUDE) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

plugins:
	$(MAKE) -C plugins all

functest: $(SOURCES)
	$(CXX) -I$(INCLUDE) $(CXXFLAGS) -DFUNCTEST -o 6502 $^ $(LDFLAGS)

run: 6502
	./6502

rebuild: clean all

clean:
	$(RM) -f 6502 plugins/*.so
