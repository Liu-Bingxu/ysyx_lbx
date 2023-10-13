#include "device.h"
#include "difftest.h"

void serial_out(word_t ch){
    // putchar(ch);
    // fflush(stdout);
    fprintf(stderr, "%c", ch);
    difftest_skip_ref();
}
