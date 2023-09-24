PLUGINS = $(shell find * -maxdepth 0 -type d)

all: $(PLUGINS)

$(PLUGINS).so: $(wildcard $@/*.cpp)
	$(MAKE) -C emu-stdio $@

clean:
	$(RM) *.so
