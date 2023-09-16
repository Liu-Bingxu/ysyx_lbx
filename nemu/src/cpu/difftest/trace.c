#include <cpu/decode.h>
#include "elf.h"
#include "utils.h"

// myitrace
#ifdef CONFIG_ITRACE
typedef struct{
    char *myinst[20];
    int mypoint_to_myinst;
}irangbuf_struct;
static irangbuf_struct irangbuf;
static char itrace[128 * 20];

void init_itrace(){
    for (int i = 0; i < 20;i++){
        irangbuf.myinst[i] = (itrace + (128 * i));
    }
    memset(itrace, '\0', (128 * 20));
    irangbuf.mypoint_to_myinst = 19;
}

void irangbuf_write(Decode *s){
    irangbuf.mypoint_to_myinst = ((irangbuf.mypoint_to_myinst + 1) % 20);
    // memset(irangbuf.myinst[irangbuf.mypoint_to_myinst], '\0', 128);
    strcpy(irangbuf.myinst[irangbuf.mypoint_to_myinst], s->logbuf);
}

void irangbuf_printf(){
    for (int i = 0; i < 20;i++){
        if(i!=irangbuf.mypoint_to_myinst){
            printf("    ");
        }
        else{
            printf("--->");
        }
        puts(irangbuf.myinst[i]);
    }
}
#endif
// myitrace

//mytrace

void init_ftrace(const char *ELF_FILE){
    if(ELF_FILE==NULL){
        IFDEF(CONFIG_FTRACE, Log(ANSI_FMT("No elf file", ANSI_BG_RED)));
        return;
    }

    FILE *elf_file = fopen(ELF_FILE,"r");
    assert(elf_file);



}

//myftrace
