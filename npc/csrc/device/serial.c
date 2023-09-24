#include "device.h"
#include "difftest.h"

void serial_out(word_t ch){
    putchar(ch);
    difftest_skip_ref();
}
