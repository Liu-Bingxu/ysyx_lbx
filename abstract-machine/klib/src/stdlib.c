#include <am.h>
#include <klib.h>
#include <klib-macros.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
static unsigned long int next = 1;

int rand(void) {
  // RAND_MAX assumed to be 32767
  next = next * 1103515245 + 12345;
  return (unsigned int)(next/65536) % 32768;
}

void srand(unsigned int seed) {
  next = seed;
}

int abs(int x) {
  return (x < 0 ? -x : x);
}

int atoi(const char* nptr) {
  int x = 0;
  while (*nptr == ' ') { nptr ++; }
  while (*nptr >= '0' && *nptr <= '9') {
    x = x * 10 + *nptr - '0';
    nptr ++;
  }
  return x;
}

void *malloc(size_t size) {
    // On native, malloc() will be called during initializaion of C runtime.
    // Therefore do not call panic() here, else it will yield a dead recursion:
    //   panic() -> putchar() -> (glibc) -> malloc() -> panic()
#if !(defined(__ISA_NATIVE__) && defined(__NATIVE_USE_KLIB__))
    static int start_flag = 0;
    static void *next_addr = NULL;
    if(start_flag==0){
        next_addr=heap.start;
        start_flag = 1;
    }
    if(size==0)return NULL;
    void *return_addr = next_addr;
    next_addr += size;
    if((uint32_t)next_addr%0x7!=0){
        next_addr -= ((uint32_t)next_addr & 0x7);
        next_addr += 0x8;
    }
    printf("addr is %x\n", return_addr);
    return return_addr;
//   panic("Not implemented");
#endif
  return NULL;
}

void free(void *ptr) {
}

#endif
