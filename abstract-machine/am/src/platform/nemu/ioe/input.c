#include <am.h>
#include <nemu.h>

#define KEYDOWN_MASK 0x8000

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
    uint32_t key = inl(KBD_ADDR);
    if(key==0){
        kbd->keydown = 0;
        kbd->keycode = AM_KEY_NONE;
    }
    else if((key&KEYDOWN_MASK)==0){
        kbd->keydown = 1;
        kbd->keycode = key;
    }
    else{
        kbd->keydown = 0;
        kbd->keycode = (key & 0x7fUL);
    }
//         kbd->keydown = 0;
//         kbd->keycode = AM_KEY_NONE;
}
