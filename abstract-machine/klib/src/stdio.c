#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

static int vsnprintf_long_num(char *out, long long OP_num, int width)
{
    char *dest = out;
    int res = 0;
    do
    {
        int w = OP_num % 10;
        (*out) = (w + 0x30);
        out++;
        res++;
        OP_num /= 10;
    } while (OP_num != 0);
    while (width > res)
    {
        (*out) = ' ';
        out++;
        res++;
    }
    out--;
    for (int i = 0; i < res / 2; i++)
    {
        char temp = (*dest);
        (*dest) = (*out);
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
        int width = 0;
        int len_flag=0;
        if (fmt[i] != '%')
        {
            (*out) = fmt[i];
            out++;
            n++;
            continue;
        }
        else{
            i++;
        }
        while((fmt[i]>=0x30)&&(fmt[i]<=0x39)){
            width *= 10;
            width += (fmt[i] - 0x30);
            i++;
        }
        while (fmt[i] == 'l')
        {
            len_flag += 1;
            i++;
        }
        if(fmt[i]=='s'){
            char *src = va_arg(ap, char *);
            assert(src != NULL);
            size_t len = strlen(src);
            while(width>len){
                (*out) = ' ';
                out++;
                n++;
                width--;
            }
            strncpy(out, src, len);
            out += len;
            n += len;
        }
        else if(fmt[i]=='d'){
            int OP_num=0;
            switch (len_flag)
            {
            case 0:
                OP_num = va_arg(ap, int);
                break;
            case 1:
                OP_num = va_arg(ap, uint64_t);
                break;
            default:
                break;
            }
            int res = vsnprintf_long_num(out, OP_num, width);
            out += res;
            n += res;
        }
        else if(fmt[i]=='c'){
            char my_char = va_arg(ap, int);
            (*out) = my_char;
            out++;
            n++;
        }
    }
    (*out) = '\0';
    return n;
    //   panic("Not implemented");
}

int printf(const char *fmt, ...) {
    assert(fmt != NULL);
    char buf[1024];
    va_list ap;
    va_start(ap, fmt);
    int res = vsprintf(buf, fmt, ap);
    va_end(ap);
    putstr(buf);
    return res;
    //   panic("Not implemented");
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
