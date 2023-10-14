#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_sdlrenderer2.h"
#include "imgui.h"
#include <iostream>

#include "cpu_debug.hpp"
#include "main_window.hpp"
#include "plugin-loader.hpp"

SDL_Window *main_window;
SDL_Renderer *main_renderer;

extern void plugin_callback_handler(PluginCallbackType, void *);

int main_window_init() {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "Failed to initialize SDL2: " << SDL_GetError() << std::endl;
    return -1;
  }

  SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");

  main_window = SDL_CreateWindow("6502emu", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720,
                                 SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
  if (!main_window) {
    std::cerr << "Failed to create SDL2 window: " << SDL_GetError() << std::endl;
    return -2;
  }

  main_renderer = SDL_CreateRenderer(main_window, -1, SDL_RENDERER_ACCELERATED);
  if (!main_renderer) {
    std::cerr << "Failed to create SDL2 renderer: " << SDL_GetError() << std::endl;
    return -2;
  }

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  ImGui_ImplSDL2_InitForSDLRenderer(main_window, main_renderer);
  ImGui_ImplSDLRenderer2_Init(main_renderer);

  return 0;
}

void main_window_update() {
  // FIXME: 4am fix; handle this more elegantly
  static bool done = false;
  if (done)
    return;

  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL2_ProcessEvent(&event);

    switch (event.type) {
      case SDL_WINDOWEVENT:
        if (!(event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(main_window)))
          break;
      case SDL_QUIT:
        plugin_callback_handler(EMU_EXIT, (void *)0);
        done = true;
        return;
    }

    for (auto plugin_ui_event : plugin_ui_event_funcs)
      plugin_ui_event(event);
  }

  SDL_SetRenderDrawColor(main_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(main_renderer);

  ImGui_ImplSDLRenderer2_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();

  render_cpu_debug_window();

  for (auto plugin_ui_render : plugin_ui_render_funcs)
    plugin_ui_render(main_renderer);

  ImGui::Render();
  ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
  SDL_RenderPresent(main_renderer);
}

void main_window_destroy() {
  ImGui_ImplSDLRenderer2_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  SDL_DestroyRenderer(main_renderer);
  SDL_DestroyWindow(main_window);
}
