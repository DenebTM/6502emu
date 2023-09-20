.PHONY: all clean 6502 plugins

all: 6502 plugins

clean:
	$(MAKE) -C ./src clean
	$(RM) -rf 6502 plugins

6502:
	$(MAKE) -C ./src $@
	mv ./src/$@ .

plugins:
	$(MAKE) -C ./src/plugins all
	mkdir -p ./plugins
	bash -c 'mv ./src/plugins/*.so ./plugins/'

rebuild: clean all
