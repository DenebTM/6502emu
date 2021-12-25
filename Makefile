CXX = g++
CXXFLAGS = -g -lm -Wall -Wextra

SOURCES = main.cpp mem.cpp cpu.cpp

6502: $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $^

.PHONY: rebuild
rebuild: clean 6502

clean:
	rm -f 6502
