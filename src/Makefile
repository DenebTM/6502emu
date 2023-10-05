CXX = g++
CXXFLAGS = -g -Wall -Wpedantic -funsigned-char -fpermissive -std=c++20 -MMD
LDLIBS = -lm -ldl -lreadline -lyaml-cpp

INCLUDE = include
OBJS = main.o sysclock.o emu-config.o cpu/cpu.o plugin-callback.o plugin-loader.o
# OBJS = main.o sysclock.o emu-config.o cpu/cpu-alt.o plugin-callback.o plugin-loader.o
OUTFILE = 6502

.PHONY: all plugins clean clean-all rebuild rebuild-plugins rebuild-all

all: 6502 plugins

$(OUTFILE): $(OBJS)
	$(CXX) -o $@ $^ $(LDLIBS)

-include $(OBJS:%.o=%.d)

%.o: %.cpp
	$(CXX) -I$(INCLUDE) $(CXXFLAGS) -c $< -o $@

plugins:
	$(MAKE) -C plugins all

clean:
	$(RM) $(OUTFILE) $(OBJS) $(OBJS:.o=.d)
clean-plugins:
	$(MAKE) -C plugins clean
clean-all: clean clean-plugins

rebuild: clean .WAIT $(OUTFILE)
rebuild-plugins: clean-plugins .WAIT plugins
rebuild-all: rebuild rebuild-plugins
