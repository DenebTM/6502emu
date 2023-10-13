#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_sdlrenderer2.h"
#include "imgui.h"
#include <iostream>

#include "main_window.hpp"

SDL_Window *main_window;
SDL_Renderer *main_renderer;

int main_window_init() {
  // commented out for now; conflicts with chardev
  // if (SDL_Init(SDL_INIT_VIDEO) < 0) {
  //   std::cerr << "Failed to initialize SDL2" << std::endl;
  //   return -1;
  // }

  // SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");

  main_window = SDL_CreateWindow("Main window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600,
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
  SDL_Event event;
  // commented out for now; conflicts with chardev
  // while (SDL_PollEvent(&event)) {
  //   ImGui_ImplSDL2_ProcessEvent(&event);
  // }

  ImGui_ImplSDLRenderer2_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();
  ImGui::ShowDemoWindow();

  SDL_SetRenderDrawColor(main_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(main_renderer);
  // etc

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
