#include <common.h>
#include "fs.h"
#include "syscall.h"
#include "proc.h"

extern PCB *current;
extern void switch_boot_pcb();

int sys_exit(int code){
    // halt(code);
    // naive_uload(NULL, "/bin/menu");
    context_uload(current, "/bin/nterm", NULL,NULL);
    switch_boot_pcb();
    yield();
    return 0;
}

int sys_yield(){
    yield();
    return 0;
}

int sys_open(const char *path,int flag,word_t mode){
    return fs_open(path, flag, mode);
}

int sys_read(int fd,void *buf,size_t count){
    return fs_read(fd, buf, count);
}

int sys_write(int fd,const void *buf,size_t count){
    return fs_write(fd, buf, count);
}

int sys_lseek(int fd, size_t offset, int whence){
    return fs_lseek(fd, offset, whence);
}

int sys_brk(intptr_t increment){
    return 0;
}

int sys_close(int fd){
    return 0;
}

int sys_gettimeofday(struct timeval *tv,void *tz){
    if(tv!=NULL){
        uint64_t us = io_read(AM_TIMER_UPTIME).us;
        tv->tv_sec = us / 1000000;
        tv->tv_usec = us % 1000000;
        return 0;
    }
    return -1;
}

int sys_execve(const char *fname, char * const argv[], char *const envp[]){
    // naive_uload(NULL, fname);
    for(int i=0;argv[i]!=NULL;i++){
        printf("argv %d is %s\n", i, argv[i]);
    }
    int res=context_uload(current, fname, argv+1, NULL);
    if(res<0)
        return -2;
    switch_boot_pcb();
    yield();
    return 0;
}

const char *sys_call_name[] = {
    TOSTRING(SYS_exit),
    TOSTRING(SYS_yield),
    TOSTRING(SYS_open),
    TOSTRING(SYS_read),
    TOSTRING(SYS_write),
    TOSTRING(SYS_kill),
    TOSTRING(SYS_getpid),
    TOSTRING(SYS_close),
    TOSTRING(SYS_lseek),
    TOSTRING(SYS_brk),
    TOSTRING(SYS_fstat),
    TOSTRING(SYS_time),
    TOSTRING(SYS_signal),
    TOSTRING(SYS_execve),
    TOSTRING(SYS_fork),
    TOSTRING(SYS_link),
    TOSTRING(SYS_unlink),
    TOSTRING(SYS_wait),
    TOSTRING(SYS_times),
    TOSTRING(SYS_gettimeofday)
};

// #define str(x) #x

void do_syscall(Context *c) {
    uintptr_t a[4];
    a[0] = c->GPR1;
    a[1] = c->GPR2;
    a[2] = c->GPR3;
    a[3] = c->GPR4;

    switch (a[0]){
    case SYS_exit:
        sys_exit(a[1]);
        break;
    case SYS_yield:
        c->GPRx=sys_yield();
        break;
    case SYS_open:
        c->GPRx = sys_open((const char *)a[1],a[2],a[3]);
        break;
    case SYS_read:
        // Log("syscall the %s with %d %d %d return the %d", sys_call_name[a[0]], a[1], a[2], a[3], c->GPRx);
        c->GPRx = sys_read(a[1], (void *)a[2], a[3]);
        break;
    case SYS_write:
        c->GPRx = sys_write(a[1], (void *)a[2], a[3]);
        // printf("Hello\n");
        // Log("syscall the %s with %d %d %d return the %d", sys_call_name[a[0]], a[1], a[2], a[3], c->GPRx);
        break;
    case SYS_lseek:
        c->GPRx = sys_lseek(a[1], a[2], a[3]);
        break;
    case SYS_brk:
        c->GPRx = sys_brk(a[1]);
        break;
    case SYS_close:
        c->GPRx = sys_close(a[1]);
        break;
    case SYS_execve:
        c->GPRx = sys_execve((const char *)a[1], (void *)a[2], (void *)a[3]);
        break;
    case SYS_gettimeofday:
        c->GPRx = sys_gettimeofday((struct timeval *)a[1], (void *)a[2]);
        break;
    default:
        panic("Unhandled syscall ID = %d", a[0]);
    }
    // Log("syscall the %s with %d %d %d return the %d", sys_call_name[a[0]], a[1], a[2], a[3], c->GPRx);
}



