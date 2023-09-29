.PHONY: all 6502 plugins clean clean-6502 clean-plugins rebuild rebuild-6502 rebuild-plugins

all: 6502 plugins

6502:
	$(MAKE) -C src $@
	cp src/$@ .

plugins:
	$(MAKE) -C src/plugins all
	mkdir -p plugins
	bash -c 'cp src/plugins/*.so plugins/'

clean: clean-6502 clean-plugins
clean-6502:
	$(RM) 6502
	$(MAKE) -C src clean
clean-plugins:
	$(RM) $(wildcard plugins/*.so)
	$(MAKE) -C src clean-plugins

rebuild: rebuild-6502 rebuild-plugins
rebuild-6502: clean-6502 rebuild-6502
rebuild-plugins: clean-plugins plugins
