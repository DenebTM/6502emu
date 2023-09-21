all: emu-stdio.so chardev.so

emu-stdio.so: $(wildcard emu-stdio/*.cpp)
	$(MAKE) -C emu-stdio $@

chardev.so: $(wildcard chardev/*.cpp)
	$(MAKE) -C chardev $@

clean:
	$(RM) -f *.so
