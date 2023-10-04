#pragma once
#include <SDL2/SDL_keyboard.h>

#include "emu-common.hpp"
#include "plugins/6520-pia.hpp"

void set_kb_row(Byte row);
Byte get_kb_row_contents();

void handle_key_down(SDL_Keysym &key);
void handle_key_up(SDL_Keysym &key);
