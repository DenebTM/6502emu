CXX = g++
CXXFLAGS = -g -O3 -Wall -Wpedantic -funsigned-char -fpermissive -std=c++20 -MMD
LDLIBS = -lm -ldl -lreadline -lyaml-cpp

INCLUDE = -Iinclude
SOURCES = main.cpp sysclock.cpp emu-config.cpp plugin-callback.cpp plugin-loader.cpp cpu/cpu.cpp ui/main_window.cpp ui/ui_thread.cpp
# SOURCES = main.cpp sysclock.cpp emu-config.cpp plugin-callback.cpp plugin-loader.cpp cpu/cpu-alt.cpp ui/main_window.cpp

# ImGui
IMGUI_DIR = ui/imgui
LDLIBS += `sdl2-config --libs` 
INCLUDE += -I$(IMGUI_DIR) `sdl2-config --cflags`
SOURCES += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_sdl2.cpp $(IMGUI_DIR)/backends/imgui_impl_sdlrenderer2.cpp

OBJDIR = obj
OBJS = $(addprefix $(OBJDIR)/, $(SOURCES:%.cpp=%.o))

OUTFILE = 6502

.PHONY: all plugins clean clean-all rebuild rebuild-plugins rebuild-all h

all: 6502 plugins

$(OUTFILE): $(OBJS)
	$(CXX) -rdynamic -o $@ $^ $(LDLIBS)

-include $(OBJS:%.o=%.d)

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(INCLUDE) $(CXXFLAGS) -c $< -o $@

plugins:
	@$(MAKE) -C plugins all

clean:
	$(RM) -r $(OUTFILE) $(OBJDIR)
clean-plugins:
	$(MAKE) -C plugins clean
clean-all: clean clean-plugins

rebuild: clean .WAIT $(OUTFILE)
rebuild-plugins: clean-plugins .WAIT plugins
rebuild-all: rebuild rebuild-plugins
