all: emu-stdio.so pet2001.so

emu-stdio.so: $(wildcard emu-stdio/*.cpp)
	$(MAKE) -C emu-stdio $@

pet2001.so: $(wildcard pet2001/*.cpp)
	$(MAKE) -C pet2001 $@

clean:
	$(RM) -f *.so
