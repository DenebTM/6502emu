PLUGINS = $(shell find * -maxdepth 0 -type d)
BUILD_TARGETS = $(PLUGINS:%=%.so)
CLEAN_TARGETS = $(PLUGINS:%=%-clean)

.PHONY: all clean $(BUILD_TARGETS) $(CLEAN_TARGETS)

all: $(BUILD_TARGETS)

$(BUILD_TARGETS):
	$(MAKE) -C $(@:.so=)

clean: $(CLEAN_TARGETS)
$(CLEAN_TARGETS):
	$(MAKE) -C $(@:-clean=) clean
