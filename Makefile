CXX = g++
CXXFLAGS = -g -funsigned-char -lm -lncurses

SOURCES = main.cpp mem.cpp cpu.cpp emu-stdio.cpp

6502: $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $^

.PHONY: rebuild
rebuild: clean 6502

clean:
	rm -f 6502