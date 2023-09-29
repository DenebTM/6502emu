CXX = g++
CXXFLAGS = -O3 -funsigned-char -fpermissive -std=c++20 -MMD
LDLIBS = -lm -ldl -lreadline -lyaml-cpp

INCLUDE = include
OBJS = main.o mem.o cpu.o plugin-callback.o emu-config.o
OUTFILE = 6502

.PHONY: all plugins clean clean-all rebuild rebuild-plugins rebuild-all

all: 6502 plugins

$(OUTFILE): $(OBJS)
	$(CXX) -o $@ $^ $(LDLIBS)

-include $(OBJS:%.o=%.d)

%.o: %.cpp
	$(CXX) -I$(INCLUDE) $(CXXFLAGS) -c $<

plugins:
	$(MAKE) -C plugins all

clean:
	$(RM) $(OUTFILE) $(OBJS) $(OBJS:.o=.d)
clean-plugins:
	$(MAKE) -C plugins clean
clean-all: clean clean-plugins

rebuild: clean $(OUTFILE)
rebuild-plugins: clean-plugins plugins
rebuild-all: rebuild rebuild-plugins
