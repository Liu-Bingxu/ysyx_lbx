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

static symbol_list symbol_tab_list={.end=NULL,.head=NULL,.node_num=0};

void symbol_list_push(symbol_list *list,char *name,word_t first_addr,word_t func_size){
    symbol_node *new=(symbol_node *)malloc(sizeof(symbol_node));
    int len=strlen(name);
    new->name=(char *)malloc(len+1);
    strcpy(new->name,name);
    new->first_addr=first_addr;
    new->end_addr=first_addr+func_size-4;
    new->prev=NULL;
    new->next=list->head;
    list->head=new;
    list->node_num++;
    if(list->node_num==1)list->end=new;
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
#endif

void init_ftrace(const char *ELF_FILE){
    if(ELF_FILE==NULL){
        Log(ANSI_FMT("No elf file", ANSI_BG_RED));
        return;
    }
    
    #ifdef CONFIG_FTRACE
    int a;

    //open file and get the ELF head
    FILE *elf_file=fopen(ELF_FILE,"rb");
    Elf32_Ehdr text;
    a=fread(&text,1,52,elf_file);
    assert(a==52);
    assert(elf_file);

    //get shstrtab head
    fseek(elf_file,text.e_shoff+text.e_shentsize*text.e_shstrndx,SEEK_SET);
    Elf32_Shdr shstrtab_sh;
    a=fread(&shstrtab_sh,text.e_shentsize,1,elf_file);
    assert(a==1);

    //get shstrtab
    fseek(elf_file,shstrtab_sh.sh_offset,SEEK_SET);
    char *shstrtab=(char *)malloc(shstrtab_sh.sh_size);
    assert(shstrtab);
    a=fread(shstrtab,shstrtab_sh.sh_size,1,elf_file);
    assert(a==1);

    //get symboltab and strtab
    Elf32_Shdr symbol_tab = {.sh_offset = 0, .sh_size = 0};
    Elf32_Shdr str_tab = {.sh_offset = 0, .sh_size = 0};
    for (int i = 0; i < text.e_shnum; i++){
        fseek(elf_file,text.e_shoff+text.e_shentsize*i,SEEK_SET);
        Elf32_Shdr shdr;
        a=fread(&shdr,text.e_shentsize,1,elf_file);
        assert(a==1);
        if (strcmp(shstrtab + shdr.sh_name, ".symtab") == 0){
            symbol_tab=shdr;
        }
        else if(strcmp(shstrtab+shdr.sh_name,".strtab")==0){
            str_tab=shdr;
        }
    }

    //get str of symbol name
    char *symbol_name=(char *)malloc(str_tab.sh_size);
    assert(symbol_name);
    fseek(elf_file, str_tab.sh_offset, SEEK_SET);
    a=fread(symbol_name,str_tab.sh_size,1,elf_file);
    assert(a ==1);

    // full search the symbol and find the FUNC
    for(int i=0;i<symbol_tab.sh_size/sizeof(Elf32_Sym);i++){
        Elf32_Sym symbol;
        fseek(elf_file,symbol_tab.sh_offset+sizeof(Elf32_Sym)*i,SEEK_SET);
        a=fread(&symbol,sizeof(Elf32_Sym),1,elf_file);
        assert(a == 1);
        if (symbol.st_info == 18){
            symbol_list_push(&symbol_tab_list,symbol_name+symbol.st_name,symbol.st_value,symbol.st_size);
        }
    }

    free(shstrtab);
    free(symbol_name);

    // //text list
    // symbol_node *now=symbol_tab_list.head;
    // for(int i=0;i<symbol_tab_list.node_num;i++){
    //     printf("FUNC: %s, Addr from %lx to %lx\n",now->name,now->first_addr,now->end_addr);
    //     now=now->next;
    // }
    #endif
    
    return;
}

//myftrace
