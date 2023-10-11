#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 0;
  while (1) {
    if((j%1000)==0)
        Log("Hello World from Nanos-lite with arg '%s' for the %dth time!", (char *)arg, j);
    j ++;
    yield();
  }
}

void context_kload(PCB *pcb, void *entry, void *arg){
    pcb->cp = kcontext((Area){.start = pcb, .end = (pcb + 1)}, entry, arg);
}
int context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]){
    int res=naive_uload(pcb, filename);
    if(res<0)
        return -1;
    void *stack = new_page(8);
    Area ustack = {
        .start = stack,
        .end = (stack + 32 * 4096),
    };
    printf("the new stack is from %p to %p\n", stack, ustack.end);
    pcb->cp->GPRx = (uintptr_t)ustack.end;
    uintptr_t argc = 1, envc = 0;
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
    int len = strlen(filename);
    memcpy(args, filename, len + 1);
    printf("argv 0 is %s\n", filename);
    (*arg) = (uintptr_t)args;
    arg++;
    args += (len + 1);
    if(argv!=NULL){
        for (int i = 0; argv[i] != NULL; i++){
            len = strlen(argv[i]);
            memcpy(args, argv[i], len + 1);
            printf("argv %d is %s\n", i+1, argv[i]);
            (*arg) = (uintptr_t)args;
            arg++;
            args += (len + 1);
        }
    }
    printf("argv finish\n");
    (*arg) = (uintptr_t)NULL;
    arg++;
    if(envp!=NULL){
        for (int i = 0; envp[i] != NULL; i++){
            len = strlen(envp[i]);
            memcpy(args, envp[i], len + 1);
            printf("envp %d is %s\n", i, envp[i]);
            (*arg) = (uintptr_t)args;
            arg++;
            args += (len + 1);
        }
    }
    (*arg) = (uintptr_t)NULL;
    arg++;
    printf("envp finish\n");
    pcb->cp->GPR2 = (uintptr_t)ustack.start;
    printf("Hello\n");
    return 0;
    // assert(0);
}

void init_proc() {
    // char *argv[] = {"0","--skip","Hello World","byebye",NULL};
    // char *envp[] = {"PATH=/bin",NULL};
    // context_kload(&pcb[0], hello_fun, "It is 1");
    // context_uload(&pcb[1], "/bin/nterm", NULL, envp);
    // switch_boot_pcb();

    Log("Initializing processes...");

    // load program here

    // yield();

    naive_uload(NULL, "/bin/audio-sdl-test");
}

Context* schedule(Context *prev) {
    current->cp = prev;
    current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
    // current = &pcb[1];
    return current->cp;
}
