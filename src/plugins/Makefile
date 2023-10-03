PLUGINS = $(shell find * -mindepth 1 -maxdepth 1 -type f -iname Makefile -printf '%h\n')
BUILD_TARGETS = $(PLUGINS:%=%.so)
CLEAN_TARGETS = $(PLUGINS:%=%-clean)

.PHONY: all clean $(BUILD_TARGETS) $(CLEAN_TARGETS)

all: $(BUILD_TARGETS)

$(BUILD_TARGETS):
	$(MAKE) -C $(@:.so=)

clean: $(CLEAN_TARGETS)
$(CLEAN_TARGETS):
	$(MAKE) -C $(@:-clean=) clean
