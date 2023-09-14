#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

static int vsnprintf_num(char *out,int OP_num){
    char *dest = out;
    int res = 0;
    do{
        int w = OP_num % 10;
        (*out) = (w + 0x30);
        out++;
        res++;
        OP_num /= 10;
    } while (OP_num != 0);
    for (int i = 0; i < res / 2; i++){
        char temp = (*dest);
        temp = (*out);
        (*out) = temp;
        dest++;
        out--;
    }
    return res;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
    assert(out != NULL);
    assert(fmt != NULL);
    int n = 0;
    for (int i = 0; fmt[i] != '\0'; i++){
        if(fmt[i]!='%'){
            (*out) = fmt[i];
            out++;
            n++;
            continue;
        }
        else{
            i++;
        }
        if(fmt[i]=='s'){
            char *src = va_arg(ap, char *);
            assert(src != NULL);
            size_t len = strlen(src);
            strncpy(out, src, len);
            out += len;
            n += len;
        }
        else if(fmt[i]=='d'){
            int OP_num = va_arg(ap, int);
            int res = vsnprintf_num(out, OP_num);
            out += res;
            n += res;
        }
    }
    (*out) = '\0';
    return n;
    //   panic("Not implemented");
}

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
    assert(out != NULL);
    assert(fmt != NULL);
    va_list ap;
    va_start(ap, fmt);
    int res = vsprintf(out, fmt, ap);
    va_end(ap);
    return res;
    //   panic("Not implemented");
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
