CXX = g++
CXXFLAGS = -O3 -funsigned-char -fpermissive -std=c++20
LDLIBS = -lm -ldl -lreadline -lyaml-cpp

INCLUDE = include
OBJS = main.o mem.o cpu.o plugin-callback.o emu-config.o
OUTFILE = 6502

.PHONY: plugins clean clean-all rebuild rebuild-plugins rebuild-all

all: 6502 plugins

$(OUTFILE): $(OBJS)
	$(CXX) -o $@ $(LDLIBS) $^ 

%.o: %.cpp
	$(CXX) -I$(INCLUDE) $(CXXFLAGS) -c $<

plugins:
	$(MAKE) -C plugins all

clean:
	$(RM) $(OUTFILE) $(OBJS)
clean-plugins:
	$(MAKE) -C plugins clean
clean-all: clean clean-plugins

rebuild: clean $(OUTFILE)
rebuild-plugins: clean-plugins plugins
rebuild-all: rebuild rebuild-plugins
