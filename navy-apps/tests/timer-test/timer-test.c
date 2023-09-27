#include<stdio.h>
#include<stdlib.h>
#include "sys/time.h"
#include "stdbool.h"
#include "assert.h"
// #include "time.h"

int main(int argc , char* argv[]){

    struct timeval tv;

    int succes = gettimeofday(&tv, NULL);

    assert(succes == 0);

    printf("Now sec is %ld, and usec is %ld\n", tv.tv_sec, tv.tv_usec);

    printf("the size of timeval is %d, the size of tv_sec is %d, the size of tv_usec is %d\n", sizeof(struct timeval), sizeof(tv.tv_sec), sizeof(tv.tv_usec));

    exit(0);
}