#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;

//interrupt request handle
Context* __am_irq_handle(Context *c) {
  //test Context
  printf("pdir = %lu \n", *(uint64_t *)(c->pdir));
  for(int i=0; i<32; i++){
    printf("gpr[%d] = %lu\n",i, *(uint64_t *)(c->gpr[i]));
  }
  printf("mcause=%lu, mstatus=%lu, mepc=%lu\n",*(uint64_t *)(c->mcause), *(uint64_t *)(c->mstatus), *(uint64_t *)(c->mepc));

  //end

  if (user_handler) {
    Event ev = {0};
    switch (c->mcause) {
      default: ev.event = EVENT_ERROR; break;
    }

    c = user_handler(ev, c);
    assert(c != NULL);
  }

  return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context*(*handler)(Event, Context*)) {
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

  // register event handler
  user_handler = handler;

  return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  return NULL;
}

void yield() {
  asm volatile("li a7, -1; ecall");
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
