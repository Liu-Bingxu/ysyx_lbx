#include "elf.h"
#include "utils.h"
#include "pmem.h"
#include "debug.h"
#include "regs.h"

// myitrace
#ifdef CONFIG_ITRACE
    typedef struct{
    char *myinst[20];
    int mypoint_to_myinst;
}irangbuf_struct;
static irangbuf_struct irangbuf;
static char itrace[128 * 20];

void init_itrace(){
    printf("Hello\n");
    for (int i = 0; i < 20;i++){
        irangbuf.myinst[i] = (itrace + (128 * i));
    }
    memset(itrace, '\0', (128 * 20));
    irangbuf.mypoint_to_myinst = 19;
}

void irangbuf_write(const char *buf){
    irangbuf.mypoint_to_myinst = ((irangbuf.mypoint_to_myinst + 1) % 20);
    // memset(irangbuf.myinst[irangbuf.mypoint_to_myinst], '\0', 128);
    strcpy(irangbuf.myinst[irangbuf.mypoint_to_myinst], buf);
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

//myftrace
#ifdef CONFIG_FTRACE
typedef struct symbol_node{
    char *name;
    word_t first_addr;
    word_t end_addr;
    struct symbol_node *prev;
    struct symbol_node *next;
}symbol_node;

typedef struct symbol_list{
    int node_num;
    symbol_node *head;
    symbol_node *end;
}symbol_list;

static symbol_list symbol_tab_list = {.node_num = 0, .head = NULL,.end = NULL};
static int func = 0;
static bool can_func_trace = false;

char *symbol_find_name(paddr_t pc,paddr_t *first_addr){
    symbol_node *now = symbol_tab_list.head;
    for (int i = 0; i < symbol_tab_list.node_num; i++){
        // printf("Hello,%d\n",symbol_tab_list.node_num);
        if ((pc >= now->first_addr) && (pc <= now->end_addr)){
            break;
        }
        now = now->next;
    }
    if(now!=NULL){
        (*first_addr) = now->first_addr;
        return now->name;
    }
    else{
        return NULL;
    }
}

void ftrce_text_jump(paddr_t pc){
    if(!can_func_trace){
        return;
    }
    char *name = NULL;
    paddr_t first_addr = 0;
    name = symbol_find_name(pc,&first_addr);
    // printf("PC is %x\n", get_gpr(32));
    Assert(name != NULL,"jump to a unkown space, PC is : "FMT_PADDR,pc);
    Assert(first_addr != 0, "jump to a unkown space, PC is : " FMT_PADDR, pc);
    if(pc!=first_addr){
        return;
    }
    Log_func(ANSI_FMT("func trace", ANSI_FG_BLUE)" "FMT_PADDR ": \t", pc);
    for (int i = 0; i < func;i++){
        Log_func("\t");
    }
    Log_func(ANSI_FMT("call",ANSI_FG_GREEN)" [");
    Log_func(ANSI_FMT("%s@"FMT_PADDR,ANSI_FG_YELLOW), name,first_addr);
    Log_func("]\n");
    func++;
    return;
}

void ftrce_text_retu(paddr_t pc){
    if(!can_func_trace){
        return;
    }
    Log_func(ANSI_FMT("func trace", ANSI_FG_BLUE)" "FMT_PADDR ": \t", pc);
    for (int i = 1; i < func;i++){
        Log_func("\t");
    }
    Log_func(ANSI_FMT("ret",ANSI_FG_GREEN)"  [");
    char *name = NULL;
    paddr_t first_addr=0;
    name = symbol_find_name(pc,&first_addr);
    Assert(name != NULL,"jump to a unkown space, PC is : " FMT_PADDR, pc);
    Assert(first_addr != 0, "jump to a unkown space, PC is : " FMT_PADDR, pc);
    // printf("Hello World\n");
    Log_func(ANSI_FMT("%s",ANSI_FG_YELLOW), name);
    Log_func("]\n");
    func--;
    return;
}

void ftrace_watch(paddr_t pc, paddr_t pc_jump){
    word_t inst;
    pmem_read(pc, &inst);
    word_t inst_mask = inst & 0x7f;
    if (inst == 0x00008067){
        ftrce_text_retu(pc);
    }
    else if((inst_mask==0x6f)||(inst_mask==0x67)){
        ftrce_text_jump(pc_jump);
    }
}

void symbol_list_push(symbol_list *list,char *name,word_t first_addr,word_t func_size){
    // printf("name is %s\n", name);
    symbol_node *_new = (symbol_node *)malloc(sizeof(symbol_node));
    int len=strlen(name);
    _new->name=(char *)malloc(len+1);
    strcpy(_new->name,name);
    _new->first_addr=first_addr;
    _new->end_addr=first_addr+func_size-4;
    _new->prev=NULL;
    _new->next=list->head;
    list->head=_new;
    list->node_num++;
    if(list->node_num==1)list->end=_new;
    can_func_trace = true;
}
void delete_symbol_list(){
    symbol_tab_list.node_num = 0;
    symbol_node *now = symbol_tab_list.head;
    while(now!=NULL){
        free(now->name);
        if(now->prev)
            free(now->prev);
        now = now->next;
    }
    free(symbol_tab_list.end);
    symbol_tab_list.end = NULL;
    symbol_tab_list.head = NULL;
}

void init_ftrace(const char *ELF_FILE){
    if(ELF_FILE==NULL){
        Log(ANSI_FMT("No elf file", ANSI_FG_RED));
        return;
    }
    
    int a;

    //open file and get the ELF head
    FILE *elf_file = fopen(ELF_FILE, "rb");
    Assert(elf_file,"can't open the ELF file: %s",ELF_FILE);
    Elf32_Ehdr text;
    a=fread(&text,1,52,elf_file);
    Assert(a=52,"error read");

    //get shstrtab head
    fseek(elf_file,text.e_shoff+text.e_shentsize*text.e_shstrndx,SEEK_SET);
    Elf32_Shdr shstrtab_sh;
    a=fread(&shstrtab_sh,text.e_shentsize,1,elf_file);
    Assert(a == 1, "error read");

    //get shstrtab
    fseek(elf_file,shstrtab_sh.sh_offset,SEEK_SET);
    char *shstrtab=(char *)malloc(shstrtab_sh.sh_size);
    Assert(shstrtab,"fail to malloc");
    a=fread(shstrtab,shstrtab_sh.sh_size,1,elf_file);
    Assert(a == 1, "error read");

    //get symboltab and strtab
    Elf32_Shdr symbol_tab = {.sh_offset = 0, .sh_size = 0};
    Elf32_Shdr str_tab = {.sh_offset = 0, .sh_size = 0};
    for (int i = 0; i < text.e_shnum; i++){
        fseek(elf_file,text.e_shoff+text.e_shentsize*i,SEEK_SET);
        Elf32_Shdr shdr;
        a=fread(&shdr,text.e_shentsize,1,elf_file);
        Assert(a == 1, "error read");
        if (strcmp(shstrtab + shdr.sh_name, ".symtab") == 0){
            symbol_tab=shdr;
        }
        else if(strcmp(shstrtab+shdr.sh_name,".strtab")==0){
            str_tab=shdr;
        }
    }

    //get str of symbol name
    char *symbol_name=(char *)malloc(str_tab.sh_size);
    Assert(symbol_name, "fail to malloc");
    fseek(elf_file, str_tab.sh_offset, SEEK_SET);
    a=fread(symbol_name,str_tab.sh_size,1,elf_file);
    Assert(a == 1, "error read");

    // full search the symbol and find the FUNC
    for(int i=0;i<symbol_tab.sh_size/sizeof(Elf32_Sym);i++){
        Elf32_Sym symbol;
        fseek(elf_file,symbol_tab.sh_offset+sizeof(Elf32_Sym)*i,SEEK_SET);
        a=fread(&symbol,sizeof(Elf32_Sym),1,elf_file);
        Assert(a == 1, "error read");
        if ((symbol.st_info&0xf) == 2){
            symbol_list_push(&symbol_tab_list,symbol_name+symbol.st_name,symbol.st_value,symbol.st_size);
        }
    }

    free(shstrtab);
    free(symbol_name);

    fclose(elf_file);

    //text list
    // symbol_node *now=symbol_tab_list.head;
    // for(int i=0;i<symbol_tab_list.node_num;i++){
    //     printf("FUNC: %s, Addr from "FMT_PADDR" to "FMT_PADDR"\n",now->name,now->first_addr,now->end_addr);
    //     now=now->next;
    // }

    if(can_func_trace)Log(ANSI_FMT("ELF file open and symbol tab OK", ANSI_FG_GREEN));
    else Log(ANSI_FMT("elf read error can't read function", ANSI_FG_RED));

    return;
}
#endif
//myftrace

// mymtrace

#ifdef CONFIG_MTRACE

void Log_mem_read(paddr_t addr){
    word_t val;
    pmem_read(addr,&val);
    Log_mem("Read  Addr: " FMT_PADDR " Data: " FMT_WORD "\n", addr, val);
}

void Log_mem_wirte(paddr_t addr,word_t data,char wmask){
    if(wmask==0x1)Log_mem("Write Addr: " FMT_PADDR " Data: " FMT_WORD "\n", addr, (data&0xff));
    else if(wmask==0x2)Log_mem("Write Addr: " FMT_PADDR " Data: " FMT_WORD "\n", addr+1, (data&0xff));
    else if(wmask==0x4)Log_mem("Write Addr: " FMT_PADDR " Data: " FMT_WORD "\n", addr+2, (data&0xff));
    else if(wmask==0x8)Log_mem("Write Addr: " FMT_PADDR " Data: " FMT_WORD "\n", addr+3, (data&0xff));
    else if(wmask==0x3)Log_mem("Write Addr: " FMT_PADDR " Data: " FMT_WORD "\n", addr, (data&0xffff));
    else if(wmask==0x6)Log_mem("Write Addr: " FMT_PADDR " Data: " FMT_WORD "\n", addr+1, (data&0xffff));
    else if(wmask==0xc)Log_mem("Write Addr: " FMT_PADDR " Data: " FMT_WORD "\n", addr+2, (data&0xffff));
    else if(wmask==0xf)Log_mem("Write Addr: " FMT_PADDR " Data: " FMT_WORD "\n", addr, data);
    else panic("error size");
}

#else

void Log_mem_read(paddr_t addr) {}
void Log_mem_wirte(paddr_t addr, word_t data, char wmask) {}

#endif

// mymtrace
