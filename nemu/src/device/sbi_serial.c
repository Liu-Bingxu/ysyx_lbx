#include <utils.h>
#include <device/map.h>
#include <SDL2/SDL.h>

#define _NUM(f) \
    f(0) f(1) f(2) f(3) f(4) f(5) f(6) f(7) f(8) f(9)
#define _A_Z(f) \
    f(A) f(B) f(C) f(D) f(E) f(F) f(G) f(H) f(I) f(J) f(K) f(L) f(M) f(N) f(O) f(P) f(Q) f(R) f(S) f(T) f(U) f(V) f(W) f(X) f(Y) f(Z)


// It's conflicted on macos with sys/_types/_key_t.h
#ifdef __APPLE__
  #undef _KEY_T 
#endif

#define SDL_KEYMAP(k) keymap[concat(SDL_SCANCODE_, k)] = *str(k);
static uint8_t keymap[256] = {};
static bool key_status;

static void init_keymap() {
    MAP(_NUM, SDL_KEYMAP)
    MAP(_A_Z, SDL_KEYMAP)
    keymap[SDL_SCANCODE_TAB] = 0x9;
    keymap[SDL_SCANCODE_SPACE] = ' ';
    keymap[SDL_SCANCODE_RETURN] = 0xD;
    keymap[SDL_SCANCODE_BACKSPACE] = 0x8;
}

#define KEY_QUEUE_LEN 1024
static int key_queue[KEY_QUEUE_LEN] = {};
static int key_f = 0, key_r = 0;

static void key_enqueue(uint8_t am_scancode){
    key_queue[key_r] = am_scancode;
    key_r = (key_r + 1) % KEY_QUEUE_LEN;
    Assert(key_r != key_f, "key queue overflow!");
}

static uint8_t key_dequeue() {
  uint8_t key = 0;
  if (key_f != key_r) {
    key = key_queue[key_f];
    key_f = (key_f + 1) % KEY_QUEUE_LEN;
    if((key >= 'A') && (key <= 'Z')){
        key = (key_status) ? (key - 'A' + 'a') : key;
    }
  }
  return key;
}

void send_key_sbi_serial(uint8_t scancode, bool is_keydown){
    if (nemu_state.state == NEMU_RUNNING && (keymap[scancode] != 0) && (!is_keydown)){
        uint8_t am_scancode = keymap[scancode];
        key_enqueue(am_scancode);
    }
    if ((scancode == SDL_SCANCODE_LSHIFT) || (scancode == SDL_SCANCODE_AC_REFRESH)){
        if(is_keydown){
            key_status = 0;
        }else{
            key_status = 1;
        }
    }
}

#define DR_OFFSET  0
#define IER_OFFSET 1
#define IIR_OFFSET 2
#define LCR_OFFSET 3
#define MCR_OFFSET 4
#define LSR_OFFSET 5
#define MSR_OFFSET 6
#define SCR_OFFSET 7

static uint8_t *sbi_serial_base = NULL;
static uint8_t sbi_serial_IER  = 0;
static uint8_t sbi_serial_divh = 0;
static uint8_t sbi_serial_divl = 0;

static void sbi_serial_putc(char ch)
{
    MUXDEF(CONFIG_TARGET_AM, putch(ch), putc(ch, stderr));
}

static void sbi_serial_io_handler(uint32_t offset, int len, bool is_write)
{
    assert(len == 1);
    if (is_write){
        switch (offset){
            case DR_OFFSET:
                if (BITS(sbi_serial_base[3], 7, 7))
                    sbi_serial_divl = sbi_serial_base[0];
                else
                    sbi_serial_putc(sbi_serial_base[0]);
                break;
            case IER_OFFSET:
                if (BITS(sbi_serial_base[3], 7, 7))
                    sbi_serial_divh = sbi_serial_base[1];
                else
                    sbi_serial_IER = sbi_serial_base[1];
                break;
            case IIR_OFFSET:
                sbi_serial_base[2] = 0xC1;
                break;
            case LCR_OFFSET:
                //? pass
                break;
            case MCR_OFFSET:
                sbi_serial_base[4] = 0x0;
                break;
            case LSR_OFFSET:
                //? pass
                break;
            case MSR_OFFSET:
                sbi_serial_base[6] = 0x0;
                break;
            case SCR_OFFSET:
                //? pass
                break;
            default:
                panic("do not support offset = %d", offset);
        }
    }else{
        switch (offset){
            case DR_OFFSET:
                if (BITS(sbi_serial_base[3], 7, 7))
                    sbi_serial_base[0] = sbi_serial_divl;
                else
                    sbi_serial_base[0] = key_dequeue();
                break;
            case IER_OFFSET:
                if (BITS(sbi_serial_base[3], 7, 7))
                    sbi_serial_base[1] = sbi_serial_divh;
                else
                    sbi_serial_base[1] = sbi_serial_IER;
                break;
            case IIR_OFFSET:
                //? pass
                break;
            case LCR_OFFSET:
                //? pass
                break;
            case MCR_OFFSET:
                //? pass
                break;
            case LSR_OFFSET:
                sbi_serial_base[5] = 0x60 | (key_f != key_r);
                break;
            case MSR_OFFSET:
                //? pass
                break;
            case SCR_OFFSET:
                //? pass
                break;
            default:
                panic("do not support offset = %d", offset);
        }
    }
}


void init_sbi_serial()
{
    sbi_serial_base = new_space(8);
    add_mmio_map("sbi_serial", CONFIG_SBI_SERIAL_MMIO, sbi_serial_base, 8, sbi_serial_io_handler);
    sbi_serial_base[3] = 0x3;
    sbi_serial_base[4] = 0x0;
    sbi_serial_base[5] = 0x60;
    sbi_serial_base[6] = 0x0;
    sbi_serial_base[7] = 0x0;
    key_status = 1;
    init_keymap();
}
