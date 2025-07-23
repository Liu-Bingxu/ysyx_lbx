#include <am.h>
#include "ysyxsoc.h"
#include <riscv/riscv.h>

#define AM_KEYS_NORMAL(_) \
    _(ESCAPE) _(F1) _(F2) _(F3) _(F4) _(F5) _(F6) _(F7) _(F8) _(F9) _(F10) _(F11) _(F12) \
    _(GRAVE) _(1) _(2) _(3) _(4) _(5) _(6) _(7) _(8) _(9) _(0) _(MINUS) _(EQUALS) _(BACKSPACE) \
    _(TAB) _(Q) _(W) _(E) _(R) _(T) _(Y) _(U) _(I) _(O) _(P) _(LEFTBRACKET) _(RIGHTBRACKET) _(BACKSLASH) \
    _(CAPSLOCK) _(A) _(S) _(D) _(F) _(G) _(H) _(J) _(K) _(L) _(SEMICOLON) _(APOSTROPHE) _(RETURN) \
    _(LSHIFT) _(Z) _(X) _(C) _(V) _(B) _(N) _(M) _(COMMA) _(PERIOD) _(SLASH) _(RSHIFT) \
    _(LCTRL) _(LALT) _(SPACE) 

#define AM_KEYS_EXT(_) _(RALT) _(RCTRL) \
    _(UP) _(DOWN) _(LEFT) _(RIGHT) _(INSERT) _(DELETE) _(HOME) _(END) _(PAGEUP) _(PAGEDOWN)

#define _NUM(f) \
    f(0) f(1) f(2) f(3) f(4) f(5) f(6) f(7) f(8) f(9)

enum {
    KEY_F9 = 1, KEY_F5 = 3, 
    KEY_F3, KEY_F1, KEY_F2, KEY_F12,
    KEY_F10 = 9,
    KEY_F8, KEY_F6, KEY_F4, KEY_TAB, KEY_GRAVE,
    KEY_LALT = 0x11, 
    KEY_LSHIFT, 
    KEY_LCTRL = 0x14, 
    KEY_Q, KEY_1,
    KEY_Z = 0x1A,
    KEY_S, KEY_A, KEY_W, KEY_2,
    KEY_C = 0x21,
    KEY_X, KEY_D, KEY_E, KEY_4, KEY_3,
    KEY_SPACE = 0x29,
    KEY_V, KEY_F, KEY_T, KEY_R, KEY_5,
    KEY_N =0x31,
    KEY_B, KEY_H, KEY_G, KEY_Y, KEY_6, 
    KEY_M = 0x3A,
    KEY_J, KEY_U, KEY_7, KEY_8,
    KEY_COMMA = 0x41,
    KEY_K, KEY_I, KEY_O, KEY_0, KEY_9, 
    KEY_PERIOD = 0x49, 
    KEY_SLASH, KEY_L, KEY_SEMICOLON, KEY_P, KEY_MINUS,
    KEY_APOSTROPHE = 0x52,
    KEY_LEFTBRACKET = 0x54,
    KEY_EQUALS = 0x55,
    KEY_CAPSLOCK = 0x58,
    KEY_RSHIFT, KEY_RETURN, KEY_RIGHTBRACKET,
    KEY_BACKSLASH = 0x5D,
    KEY_BACKSPACE = 0x66,
    KEY_ESCAPE = 0x76,
    KEY_F11 = 0x78,
    KEY_F7 = 0x83,
};

enum{
    KEY__1 = 0x69,
    KEY__4 = 0x6B, KEY__7,
    KEY__0 = 0x70, 
    KEY__2 = 0x72,
    KEY__5, KEY__6, KEY__8,
    KEY__3 = 0x7A,
    KEY__9 = 0x7D,
};

enum{
    KEY_ext_RALT = 0x11,
    KEY_ext_RCTRL = 0x14,
    KEY_ext_END = 0x69,
    KEY_ext_LEFT = 0x6B,
    KEY_ext_HOME = 0x6C,
    KEY_ext_INSERT = 0x70,
    KEY_ext_DELETE, KEY_ext_DOWN, 
    KEY_ext_RIGHT = 0x74,
    KEY_ext_UP = 0x75,
    KEY_ext_PAGEDOWN = 0x7A,
    KEY_ext_PAGEUP = 0x7D,
};

#define AM_MAKE2NUM_NORMAL(key)      \
    case KEY_##key:                  \
        kbd->keycode = AM_KEY_##key; \
        break;

#define AM_MAKE2NUM_NUM(key)         \
    case KEY__##key:                 \
        kbd->keycode = AM_KEY_##key; \
        break;

#define AM_MAKE2NUM_EXT(key)         \
    case KEY_ext_##key:              \
        kbd->keycode = AM_KEY_##key; \
        break;

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
    uint8_t key = inb(PS2_KEY_ADDR);
    bool down_flag = true;
    kbd->keydown = true;
re_get_break_code:
    switch (key){
        _NUM(AM_MAKE2NUM_NUM)
        AM_KEYS_NORMAL(AM_MAKE2NUM_NORMAL)
        case 0xE0:
            goto get_next;
        case 0xF0:
            kbd->keydown = false;
            down_flag = false;
            goto get_next;
        default:
            kbd->keydown = false;
            kbd->keycode = AM_KEY_NONE;
            break;
    }
    return;

get_next:
    do{
        key = inb(PS2_KEY_ADDR);
    }while(key == 0x0);
    if(down_flag == false)
        goto re_get_break_code;

    switch (key){
        AM_KEYS_EXT(AM_MAKE2NUM_EXT)
        case 0xF0:
            kbd->keydown = false;
            goto get_next;
        default:
            kbd->keydown = false;
            kbd->keycode = AM_KEY_NONE;
            break;
    }
}
