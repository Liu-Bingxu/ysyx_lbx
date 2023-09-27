#include<stdio.h>
#include<stdlib.h>
#include "sys/time.h"
#include "stdbool.h"
#include "assert.h"
#include "stdint.h"
// #include "time.h"

int main(int argc , char* argv[]){

    struct timeval tv,tv_now;

    int succes = gettimeofday(&tv, NULL);

    assert(succes == 0);

    printf("Now sec is %ld, and usec is %ld\n", tv.tv_sec, tv.tv_usec);

    printf("the size of timeval is %d, the size of tv_sec is %d, the size of tv_usec is %d\n", sizeof(struct timeval), sizeof(tv.tv_sec), sizeof(tv.tv_usec));

    uint64_t times = 1;

    while(1){
        assert(gettimeofday(&tv_now, NULL) == 0);
        if(((tv_now.tv_sec*10)-(tv.tv_sec*10))<5){
            continue;
        }
        tv = tv_now;
        printf("Hello world!! It is the %lld times\n", times);
        
        times++;
    }

    exit(0);
}