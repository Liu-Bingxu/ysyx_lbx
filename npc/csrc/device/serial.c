#include "device.h"
#include "difftest.h"

void serial_out(char ch){
    // putchar(ch);
    // fflush(stdout);
    fprintf(stderr, "%c", ch);
    // difftest_skip_ref();
}
