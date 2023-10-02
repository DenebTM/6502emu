#pragma once
#include <SDL2/SDL_keycode.h>
#include <map>
#include <vector>

const std::map<SDL_Keycode, std::pair<int, int>> shiftmap = {
    {SDLK_COMMA, {9, 0x08}},     // <
    {SDLK_PERIOD, {8, 0x10}},    // >
    {SDLK_QUOTE, {8, 0x02}},     // @
    {SDLK_EQUALS, {7, 0x80}},    // +
    {SDLK_SLASH, {7, 0x10}},     // ?
    {SDLK_8, {5, 0x80}},         // *
    {SDLK_SEMICOLON, {5, 0x10}}, // :
    {SDLK_6, {2, 0x20}},         // ^
    {SDLK_0, {1, 0x10}},         // )
    {SDLK_4, {1, 0x02}},         // "
    {SDLK_2, {1, 0x01}},         // "
    {SDLK_9, {0, 0x10}},         // (
    {SDLK_7, {0, 0x08}},         // &
    {SDLK_5, {0, 0x04}},         // %
    {SDLK_1, {0, 0x01}},         // !
};

const std::map<SDL_Keycode, std::pair<int, int>> ctrlmap = {
    {SDLK_c, {9, 0x10}}, // STOP
    {SDLK_r, {9, 0x01}}, // Reverse On
};

const std::map<SDL_Keycode, std::vector<std::pair<int, int>>> keymap = {
    {SDLK_EQUALS, {{9, 0x80}}},       // =
    {SDLK_PERIOD, {{9, 0x40}}},       // .
    {SDLK_SPACE, {{9, 0x04}}},        // space
    {SDLK_LEFTBRACKET, {{9, 0x02}}},  // [
    {SDLK_0, {{8, 0x40}}},            // 0
    {SDLK_RSHIFT, {{8, 0x20}}},       // rshift
    {SDLK_RIGHTBRACKET, {{8, 0x04}}}, // ]
    {SDLK_LSHIFT, {{8, 0x01}}},       // lshift
    {SDLK_MINUS, {{8, 0x80}}},        // -
    {SDLK_2, {{7, 0x40}}},            // 2
    {SDLK_COMMA, {{7, 0x08}}},        // ,
    {SDLK_n, {{7, 0x04}}},            // n
    {SDLK_v, {{7, 0x02}}},            // v
    {SDLK_x, {{7, 0x01}}},            // x
    {SDLK_3, {{6, 0x80}}},            // 1
    {SDLK_1, {{6, 0x40}}},            // 1
    {SDLK_RETURN, {{6, 0x20}}},       // return
    {SDLK_SEMICOLON, {{6, 0x10}}},    // ;
    {SDLK_m, {{6, 0x08}}},            // m
    {SDLK_b, {{6, 0x04}}},            // b
    {SDLK_c, {{6, 0x02}}},            // c
    {SDLK_z, {{6, 0x01}}},            // z
    {SDLK_5, {{5, 0x40}}},            // 5
    {SDLK_k, {{5, 0x08}}},            // k
    {SDLK_h, {{5, 0x04}}},            // h
    {SDLK_f, {{5, 0x02}}},            // f
    {SDLK_s, {{5, 0x01}}},            // s
    {SDLK_6, {{4, 0x80}}},            // 6
    {SDLK_4, {{4, 0x40}}},            // 4
    {SDLK_l, {{4, 0x10}}},            // l
    {SDLK_j, {{4, 0x08}}},            // j
    {SDLK_g, {{4, 0x04}}},            // g
    {SDLK_d, {{4, 0x02}}},            // d
    {SDLK_a, {{4, 0x01}}},            // a
    {SDLK_SLASH, {{3, 0x80}}},        // /
    {SDLK_8, {{3, 0x40}}},            // 8
    {SDLK_p, {{3, 0x10}}},            // p
    {SDLK_i, {{3, 0x08}}},            // i
    {SDLK_y, {{3, 0x04}}},            // y
    {SDLK_r, {{3, 0x02}}},            // r
    {SDLK_w, {{3, 0x01}}},            // w
    {SDLK_9, {{2, 0x80}}},            // 9
    {SDLK_7, {{2, 0x40}}},            // 7
    {SDLK_o, {{2, 0x10}}},            // o
    {SDLK_u, {{2, 0x08}}},            // u
    {SDLK_t, {{2, 0x04}}},            // t
    {SDLK_e, {{2, 0x02}}},            // e
    {SDLK_q, {{2, 0x01}}},            // q
    {SDLK_BACKSPACE, {{1, 0x80}}},    // backspace (+shift=insert)
    {SDLK_DOWN, {{1, 0x40}}},         // down (+shift=up)
    {SDLK_BACKSLASH, {{1, 0x08}}},    // backslash
    {SDLK_QUOTE, {{1, 0x04}}},        // '
    {SDLK_RIGHT, {{0, 0x80}}},        // right (+shift=left)
    {SDLK_HOME, {{0, 0x40}}},         // home (+shift=cls)
    {SDLK_HASH, {{0, 0x02}}},         // #

    // inputs emulating a shift press
    {SDLK_UP, {{8, 0x01}, {1, 0x40}}},     // up (=shift+down)
    {SDLK_LEFT, {{8, 0x01}, {0, 0x80}}},   // left (=shift+right)
    {SDLK_INSERT, {{8, 0x01}, {1, 0x80}}}, // insert (=shift+backspace)
};
