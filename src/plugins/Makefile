PLUGINS = $(shell find * -maxdepth 0 -type d)

.PHONY: all clean $(PLUGINS) $(PLUGINS:%=%-clean)

all: $(PLUGINS)

$(PLUGINS):
	$(MAKE) -C $@ ../$@.so

clean: $(PLUGINS:%=%-clean)
$(PLUGINS:%=%-clean):
	$(MAKE) -C $(@:-clean=) clean
