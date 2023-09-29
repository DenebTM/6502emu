PLUGINS = $(patsubst %,%.so,$(shell find * -maxdepth 0 -type d))

.PHONY: all clean

all: $(PLUGINS)

%.so: %
	$(MAKE) -C $< ../$@

clean: $(patsubst %.so,%-clean,$(PLUGINS))
%-clean: %
	$(MAKE) -C $< clean
