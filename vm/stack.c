#include "userprog/exception.h"
#include <inttypes.h>
#include <stdio.h>
#include "userprog/gdt.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "vm/page.h"
#include "vm/frame.h"

void stack_growth(void **esp, bool present, bool access, bool accessor, void* address) {
  if (address != esp - 32) { //Check that the access is not below esp - 32
    if (present && access && address != esp - 32) { //If the page does not exist and the operation is a write
	  uint32_t bytes_to_allocate = address - esp; //Find the number of bytes to allocate for the new value to be written to the stack
  	  struct thread* current = thread_current(); //Get the address of the current thread
      struct supplement_page_table_elem* spe = page_add_supplemental_elem(current->spt, current, address, false); //Allocate a new spot on the spt for the data to be written to the stack
	  frame_request(spe); //Now request a new frame to be added also
    }
	if (present && !accessor && access) { 
	  //If a fault occurres in the kernel
	}
  }
}
