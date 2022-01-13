CXX = g++
CXXFLAGS = -g -funsigned-char -lm -lncurses

SOURCES = main.cpp common.h mem.cpp cpu.cpp emu-stdio.cpp

6502: $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $^

run: 6502
	./6502

.PHONY: rebuild
rebuild: clean 6502

clean:
	rm -f 6502