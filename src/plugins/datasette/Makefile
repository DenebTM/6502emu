LDLIBS =
SOURCES = main.cpp cassette.cpp
OUTFILE = ../datasette.so

# ImGui
IMGUI_DIR = ../../../lib/imgui
INCLUDE += -I$(IMGUI_DIR)

# Native File Dialog Extended
NFDX_DIR = ../../../lib/nfdx/src
LDLIBS += `pkg-config --libs dbus-glib-1`
INCLUDE += -I$(NFDX_DIR)/include `pkg-config --cflags dbus-glib-1`
SOURCES += $(NFDX_DIR)/nfd_portal.cpp

include ../plugin-makefile-base
