#ifndef VM_STACK_H
#include "userprog/exception.h"
#include <inttypes.h>
#include <stdio.h>
#include "userprog/gdt.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "vm/page.h"
#include "vm/frame.h"

#define MAX_STACK_SIZE 0x800000

void stack_growth(void *esp, bool present, bool access, bool accessor, void* address);
#endif
