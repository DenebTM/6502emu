LDLIBS =
SOURCES = main.cpp cassette.cpp
OUTFILE = ../datasette.so

# ImGui
IMGUI_DIR = ../../ui/imgui
INCLUDE += -I$(IMGUI_DIR)

# Native File Dialog Extended
LDLIBS += `pkg-config --libs dbus-glib-1`
INCLUDE += -Infdx/src/include `pkg-config --cflags dbus-glib-1`
SOURCES += nfdx/src/nfd_portal.cpp

include ../plugin-makefile-base
