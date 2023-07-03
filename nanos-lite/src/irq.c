#include <common.h>
#include "syscall.h"

#define INC_PC c->mepc += 4

static Context* do_event(Event e, Context* c) {
  //printf("EVENT NO: %d\n", e.event);
  switch (e.event) {
    case EVENT_SYSCALL: do_syscall(c); INC_PC; break;
    case EVENT_YIELD: printf("this is EVENT_yield!\n"); INC_PC;break;
    default: panic("Unhandled event ID = %d", e.event);
  }

  return c;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
