#include "userprog/exception.h"
#include <inttypes.h>
#include <stdio.h>
#include "userprog/gdt.h"
#include "userprog/syscall.h"
#include "userprog/pagedir.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "vm/page.h"
#include "vm/frame.h"
#include "vm/stack.h"

void stack_growth(void *esp, bool present, bool access, bool accessor, void* address) {
  bool stack_access, valid_access, stack_growth; //Flags that indicate if the access was a stack access, if it is a valid one, and if the stack needs to grow
  stack_access = (address <= PHYS_BASE && address >= esp - 32); //Check if the access was to a place between PHYS_BASE and before esp-32 (any access beyond that is too far, according to Stanford)
  stack_growth = (esp <= address); //If you are accessing a point that is below the stack pointer, then the stack needs to grow
  valid_access = (PHYS_BASE - MAX_STACK_SIZE <= address); //Now check if the address points to a space between the base of the stack and its maximum size

  if (stack_access && stack_growth && valid_access) { //If all the above are true, we must grow the stack
	uint32_t bytes_to_allocate = esp - address; //Find the number of bytes to allocate for the new value to be written to the stack
  	struct thread* current = thread_current(); //Get the address of the current thread
    struct supplement_page_table_elem* spe = page_add_supplemental_elem(&current->spt, current, address, false); //Allocate a new spot on the spt for the data to be written to the stack
	frame_request(spe); //Now request a new frame to be added also
  }

  return; //Now, the new page has been allocated and we can return
}
