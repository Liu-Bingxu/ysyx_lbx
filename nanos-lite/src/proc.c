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
void context_uload(PCB *pcb,void *filename){
    naive_uload(pcb, filename);
}

void init_proc() {
    context_kload(&pcb[0], hello_fun, "It is 1");
    context_uload(&pcb[1], "/bin/pal");
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
