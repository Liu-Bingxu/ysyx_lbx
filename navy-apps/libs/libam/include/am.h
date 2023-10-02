#ifndef __AM_H__
#define __AM_H__

#include "klib-macros.h"
#if defined(static_assert)
#undef static_assert
#endif
#define ARCH_H "navy.h"
#include "am-origin.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "amdev.h"

#endif
