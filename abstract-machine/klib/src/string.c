#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
    size_t len=0;
    while (s[len]!='\0'){
        len++;
    }
    return len;
    //   panic("Not implemented");
}

char *strcpy(char *dst, const char *src) {
    char *res = dst;
    while((*src)!='\0'){
        (*dst) = (*src);
        dst++;
        src++;
    }
    (*dst) = '\0';
    return res;
    //   panic("Not implemented");
}

char *strncpy(char *dst, const char *src, size_t n) {
    char *res=dst;
    for (; n > 0;){
        n--;
        (*dst) = (*src);
        dst++;
        if ((*src) != '\0'){
            src++;
        }
        else{
            break;
        }
    }
    for (; n > 0;n--){
        (*dst) = '\0';
        dst++;
    }
    return res;
    //   panic("Not implemented");
}

char *strcat(char *dst, const char *src) {
    char *res = dst;
    while((*dst)!='\0'){
        dst++;
    }
    while((*src)!='\0'){
        (*dst) = (*src);
        dst++;
        src++;
    }
    (*dst) = '\0';
    return res;
    //   panic("Not implemented");
}

int strcmp(const char *s1, const char *s2) {
    int res = ((*s1)-(*s2));
    while((res==0)&&((*s1)!='\0')){
        s1++;
        s2++;
        res = ((*s1) - (*s2));
    }
    return res;
    //   panic("Not implemented");
}

int strncmp(const char *s1, const char *s2, size_t n) {
    if(n==0){
        return 0;
    }
    int cnt = 0;
    int res = ((*s1) - (*s2));
    while((res==0)&&((*s1)!='\0')){
        s1++;
        s2++;
        res = ((*s1) - (*s2));
        cnt++;
        if(cnt==n){
            return 0;
        }
    }
    return res;
//   panic("Not implemented");
}

void *memset(void *s, int c, size_t n) {
    word_t *dst = s;
    word_t mychar = 0;
    for (int i = 0; i < sizeof(word_t); i++){
        mychar = mychar << 8;
        mychar |= (char)c;
    }
    size_t main_cnt = n / sizeof(word_t);
    size_t second_cnt = n % sizeof(word_t);
    for (; main_cnt > 0; main_cnt--){
        (*dst) = mychar;
        dst++;
    }
    for (int i = 0; i < second_cnt; i++){
        *(((char *)dst) + i) = (char)c;
    }
    return s;
    //   panic("Not implemented");
}
 
void *memcpy(void *out, const void *in, size_t n) {
    word_t *dest = out;
    const word_t *src = in;
    word_t main_cnt = n / sizeof(word_t);
    word_t second_cnt = n % sizeof(word_t);
    for (; main_cnt > 0;main_cnt--){
        (*dest) = (*src);
        dest++;
        src++;
    }
    for (int i = 0; i < second_cnt;i++){
        *(((char *)dest) + i) = *(((char *)src) + i);
    }
    return out;
    //   panic("Not implemented");
}

void *memmove(void *dst, const void *src, size_t n) {
    if(dst!=src){
        if ((src<dst)&&((src+n)>dst)){
            int first = src + n - dst;
            int second = n - first;
            memcpy((dst + first), (src + first), first);
            memcpy(dst, src, second);
        }
        else{
            memcpy(dst, src, n);
        }
    }
    return dst;
    //   panic("Not implemented");
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const word_t *src1 = s1;
    const word_t *src2 = s2;
    word_t main_cnt = n / sizeof(word_t);
    word_t second_cnt = n % sizeof(word_t);
    int res = 0;
    for (; main_cnt > 0; main_cnt--){
        if((*src1)==(*src2)){
            src1++;
            src2++;
        }
        else{
            for (int i = 0; i < sizeof(word_t);i++){
                res = ((*(((char *)src1) + i)) - (*(((char *)src2) + i)));
                if(res!=0){
                    return res;
                }
            }
        }
    }
    for (int i = 0; i < second_cnt;i++){
        res = ((*(((char *)src1) + i)) - (*(((char *)src2) + i)));
        if(res!=0){
            return res;
        }
    }
    return res;
    //   panic("Not implemented");
}

#endif
