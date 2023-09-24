all: emu-stdio.so chardev.so pia1.so

emu-stdio.so: $(wildcard emu-stdio/*.cpp)
	$(MAKE) -C emu-stdio $@

chardev.so: $(wildcard chardev/*.cpp)
	$(MAKE) -C chardev $@

pia1.so: $(wildcard pia1/*.cpp)
	$(MAKE) -C pia1 $@

clean:
	$(RM) -f *.so
