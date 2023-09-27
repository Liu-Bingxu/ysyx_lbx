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

    printf("Now sec is %d, and usec is %d\n", tv.tv_sec, tv.tv_usec);

    exit(0);
}