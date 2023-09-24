#ifndef __DEVICE_H__
#define __DEVICE_H__

#include "common.h"

#define DEVICE_ADDR     0xa0000000
#define SERIAL_ADDR     (DEVICE_ADDR+0x0)

void serial_out(word_t ch);

#endif