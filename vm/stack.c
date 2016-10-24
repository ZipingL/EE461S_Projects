#include "userprog/exception.h"
#include <inttypes.h>
#include <stdio.h>
#include "userprog/gdt.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "vm/page.h"
#include "vm/frame.h"
#include "userprog/process.h"


void stack_growth(struct intr_frame * f, bool present, bool access, bool accessor, void* uva) {
struct thread *t = thread_current();
uint8_t* esp = accessor == true ? f->esp : t->esp;
 if ((uint8_t*)uva >= (uint8_t*) esp - 32) { //Check that the access is not below esp - 32
  //  if (present && access && address != esp - 32) { //If the page does not exist and the operation is a write
	//  int32_t bytes_to_allocate = (int) uva - (int) esp; //Find the number of bytes to allocate for the new value to be written to the stack

			struct thread* current = thread_current(); //Get the address of the current thread
      uint8_t* uva_masked =  (uint32_t) uva & (uint32_t)0xFFFFF000;
      struct supplement_page_table_elem* spe = page_add_supplemental_elem(&current->spt, current, uva_masked, false); //Allocate a new spot on the spt for the data to be written to the stack
			spe->writable = true;
      spe->vaddr = uva_masked;
      spe->pin = true;

			uint8_t* kp = frame_request(spe);

      if(kp != NULL)
        install_page (uva_masked, kp, true);

      // WE now need to swap something out
      else{
			  kp = frame_swap_for_new(spe);
      }
      spe->in_swap = false;
    //}
	//if (present && !accessor && access) {
	  //If a fault occurres in the kernel
	//}
 }
 else { // Or else exit if the access to a stack location was invalid, not missing
//printf("%p\n", uva);
   exit(-1);
 }
}
