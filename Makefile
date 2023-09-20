CXX = g++
CXXFLAGS = -g -funsigned-char -std=c++23
LDFLAGS = -lncurses -lm -ldl

SOURCES = main.cpp mem.cpp cpu.cpp

6502: $(SOURCES) plugins/emu-stdio.so
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

plugins/emu-stdio.so:
	$(MAKE) -C plugins/emu-stdio emu-stdio.so

functest: $(SOURCES)
	$(CXX) $(CXXFLAGS) -DFUNCTEST -o 6502 $^ $(LDFLAGS)

run: 6502
	./6502

.PHONY: rebuild
rebuild: clean 6502

clean:
	rm -f 6502 plugins/*.so
