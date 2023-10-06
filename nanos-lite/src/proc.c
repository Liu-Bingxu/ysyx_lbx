#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with arg '%s' for the %dth time!", (char *)arg, j);
    j ++;
    yield();
  }
}

void context_kload(PCB *pcb, void *entry, void *arg){
    pcb->cp = kcontext((Area){.start = pcb, .end = (pcb + 1)}, entry, arg);
}
void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]){
    naive_uload(pcb, filename);
    Area ustack = heap;
    pcb->cp->GPRx = (uintptr_t)ustack.end;
    uintptr_t argc = 0, envc = 0;
    if(argv!=NULL){
        for (int i = 0; argv[i] != NULL; i++){
            argc++;
        }
    }
    if(envp!=NULL){
        for (int i = 0; envp[i] != NULL; i++){
            envc++;
        }
    }
    char *args = (char *)(ustack.start + (argc + envc + 13) * sizeof(uintptr_t));
    uintptr_t *arg = (uintptr_t *)ustack.start;
    (*arg) = argc;
    arg++;
    if(argv!=NULL){
        for (int i = 0; argv[i] != NULL; i++){
            int len = strlen(argv[i]);
            memcpy(args, argv[i], len + 1);
            (*arg) = (uintptr_t)args;
            arg++;
            args += (len + 1);
        }
    }
    (*arg) = (uintptr_t)NULL;
    arg++;
    if(envp!=NULL){
        for (int i = 0; envp[i] != NULL; i++){
            int len = strlen(envp[i]);
            memcpy(args, envp[i], len + 1);
            (*arg) = (uintptr_t)args;
            arg++;
            args += (len + 1);
        }
    }
    (*arg) = (uintptr_t)NULL;
    arg++;
    pcb->cp->GPR2 = (uintptr_t)ustack.start;
}

void init_proc() {
    char *argv[] = {"NULL"};
    context_kload(&pcb[0], hello_fun, "It is 1");
    context_uload(&pcb[1], "/bin/pal",argv,NULL);
    switch_boot_pcb();

    Log("Initializing processes...");

    // load program here

    yield();

    naive_uload(NULL, "/bin/bird");
}

Context* schedule(Context *prev) {
    current->cp = prev;
    current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
    return current->cp;
}
