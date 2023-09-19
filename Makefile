CXX = g++
CXXFLAGS = -g -funsigned-char -lm -lncurses -std=c++23

SOURCES = main.cpp mem.cpp cpu.cpp emu-stdio.cpp

6502: $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $^

functest: $(SOURCES)
	$(CXX) $(CXXFLAGS) -DFUNCTEST -o 6502 $^

run: 6502
	./6502

.PHONY: rebuild
rebuild: clean 6502

clean:
	rm -f 6502
