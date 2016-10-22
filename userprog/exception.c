#include "userprog/exception.h"
#include <inttypes.h>
#include <stdio.h>
#include "userprog/gdt.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "vm/page.h"
#include "devices/block.h"
#include "userprog/syscall.h"
#include "userprog/process.h"
#define STACK_SIZE 8388608
/* Number of page faults processed. */
static long long page_fault_cnt;

static void kill (struct intr_frame *, struct supplement_page_table_elem *);
static void page_fault (struct intr_frame *);


bool exception_check_if_stack(uint8_t * fault_addr)
{
  //printf("hello3%d\n", ((int) PHYS_BASE) - (int) fault_addr);
  if(( ((int) PHYS_BASE) - (int) fault_addr) <
            ( (uint32_t ) STACK_SIZE))
            return true;
  else
  return false;
}

/* Registers handlers for interrupts that can be caused by user
   programs.

   In a real Unix-like OS, most of these interrupts would be
   passed along to the user process in the form of signals, as
   described in [SV-386] 3-24 and 3-25, but we don't implement
   signals.  Instead, we'll make them simply kill the user
   process.

   Page faults are an exception.  Here they are treated the same
   way as other exceptions, but this will need to change to
   implement virtual memory.

   Refer to [IA32-v3a] section 5.15 "Exception and Interrupt
   Reference" for a description of each of these exceptions. */
void
exception_init (void)
{
  /* These exceptions can be raised explicitly by a user program,
     e.g. via the INT, INT3, INTO, and BOUND instructions.  Thus,
     we set DPL==3, meaning that user programs are allowed to
     invoke them via these instructions. */
  intr_register_int (3, 3, INTR_ON, kill, "#BP Breakpoint Exception");
  intr_register_int (4, 3, INTR_ON, kill, "#OF Overflow Exception");
  intr_register_int (5, 3, INTR_ON, kill,
                     "#BR BOUND Range Exceeded Exception");

  /* These exceptions have DPL==0, preventing user processes from
     invoking them via the INT instruction.  They can still be
     caused indirectly, e.g. #DE can be caused by dividing by
     0.  */
  intr_register_int (0, 0, INTR_ON, kill, "#DE Divide Error");
  intr_register_int (1, 0, INTR_ON, kill, "#DB Debug Exception");
  intr_register_int (6, 0, INTR_ON, kill, "#UD Invalid Opcode Exception");
  intr_register_int (7, 0, INTR_ON, kill,
                     "#NM Device Not Available Exception");
  intr_register_int (11, 0, INTR_ON, kill, "#NP Segment Not Present");
  intr_register_int (12, 0, INTR_ON, kill, "#SS Stack Fault Exception");
  intr_register_int (13, 0, INTR_ON, kill, "#GP General Protection Exception");
  intr_register_int (16, 0, INTR_ON, kill, "#MF x87 FPU Floating-Point Error");
  intr_register_int (19, 0, INTR_ON, kill,
                     "#XF SIMD Floating-Point Exception");

  /* Most exceptions can be handled with interrupts turned on.
     We need to disable interrupts for page faults because the
     fault address is stored in CR2 and needs to be preserved. */
  intr_register_int (14, 0, INTR_OFF, page_fault, "#PF Page-Fault Exception");
}

/* Prints exception statistics. */
void
exception_print_stats (void)
{
  printf ("Exception: %lld page faults\n", page_fault_cnt);
}

/* Handler for an exception (probably) caused by a user process. */
static void
kill (struct intr_frame *f, struct supplement_page_table_elem *spe)
{
  printf("Page info:\n Program name: %s\n %s\n %s\n %s\n", spe->t->full_name,
  spe->executable_page == true ? "is executable_page" : "not exectuable_page",
  spe->in_filesys == true ? "in filesys" : spe->in_swap == true ? "in swap" : "in frame",
  spe->access == true ? "accessed before" : "never accessed");
  /* This interrupt is one (probably) caused by a user process.
     For example, the process might have tried to access unmapped
     virtual memory (a page fault).  For now, we simply kill the
     user process.  Later, we'll want to handle page faults in
     the kernel.  Real Unix-like operating systems pass most
     exceptions back to the process via signals, but we don't
     implement them. */

  /* The interrupt frame's code segment value tells us where the
     exception originated. */
  switch (f->cs)
    {
    case SEL_UCSEG:
      /* User's code segment, so it's a user exception, as we
         expected.  Kill the user process.  */
      printf ("%s: dying due to interrupt %#04x (%s).\n",
              thread_name (), f->vec_no, intr_name (f->vec_no));
      intr_dump_frame (f);
      exit (-1);

    case SEL_KCSEG:
      /* Kernel's code segment, which indicates a kernel bug.
         Kernel code shouldn't throw exceptions.  (Page faults
         may cause kernel exceptions--but they shouldn't arrive
         here.)  Panic the kernel to make the point.  */
      intr_dump_frame (f);
      PANIC ("Kernel bug - unexpected interrupt in kernel");

    default:
      /* Some other code segment?  Shouldn't happen.  Panic the
         kernel. */
      printf ("Interrupt %#04x (%s) in unknown segment %04x\n",
             f->vec_no, intr_name (f->vec_no), f->cs);
      exit(-1);
    }
}

/* Page fault handler.  This is a skeleton that must be filled in
   to implement virtual memory.  Some solutions to project 2 may
   also require modifying this code.

   At entry, the address that faulted is in CR2 (Control Register
   2) and information about the fault, formatted as described in
   the PF_* macros in exception.h, is in F's error_code member.  The
   example code here shows how to parse that information.  You
   can find more information about both of these in the
   description of "Interrupt 14--Page Fault Exception (#PF)" in
   [IA32-v3a] section 5.15 "Exception and Interrupt Reference". */
static void
page_fault (struct intr_frame *f)
{
  bool not_present;  /* True: not-present page, false: writing r/o page. */
  bool write;        /* True: access was write, false: access was read. */
  bool user;         /* True: access by user, false: access by kernel. */
  void *fault_addr;  /* Fault address. */

  /* Obtain faulting address, the virtual address that was
     accessed to cause the fault.  It may point to code or to
     data.  It is not necessarily the address of the instruction
     that caused the fault (that's f->eip).
     See [IA32-v2a] "MOV--Move to/from Control Registers" and
     [IA32-v3a] 5.15 "Interrupt 14--Page Fault Exception
     (#PF)". */
  asm ("movl %%cr2, %0" : "=r" (fault_addr));


  /* Turn interrupts back on (they were only off so that we could
     be assured of reading CR2 before it changed). */
  intr_enable ();

  /* Count page faults. */
  page_fault_cnt++;

  /* Determine cause. */
  not_present = (f->error_code & PF_P) == 0;
  write = (f->error_code & PF_W) != 0;
  user = (f->error_code & PF_U) != 0;


  /* Exit immediately if the fault_addr is kerenl virtual address */
  /* We have to make sure the user was trying to access user virtual address
     the user should never access kernel virtual addresses */
  /* Why are we doing this? Well because of the way I handled tests
     that tried to access memory addresses at the kernel level, I
     forced a page fault to happen, see syscall.c for implementation*/
  if(!is_user_vaddr(fault_addr) && user)
  {
     exit(-1);
   }

    uint8_t* uva = (uint32_t)fault_addr & (uint32_t)0xFFFFF000;

    struct supplement_page_table_elem* spe =
    page_find_spe(uva);
    if(spe == NULL && !exception_check_if_stack(fault_addr))
    {
      exit(-1);
    }

    // Here we do access checks

    // Exit if user tries to write to page that contains code
    // Note that the user can read the code page
    // What do the checks (the if statment) mean?
    // Explained In Order
    // Check if its a user doing it
    // CHeck if the page is truly code page
    // Check if the user was trying the write to it
    // Check if the page is loaded in frame, meaning
    // A user canwrite to a code page, if and only if
    // its the case such that we are actually trying to write in the code
    // into the page.
    // We do not want the user to write to the code page,
    // if there is no reason to. Plus, it lets us pass the tests
    // that tries to do so.
/*    if( spe->executable_page == true && write && user )
    {
      // If this fails, then something is wrong!
      //printf("spe->in_filesys %d\n%s\n", spe->in_filesys, spe->t->full_name);
      printf ("fakePage fault at %p: %s error %s page in %s context.\n",
              fault_addr,
              not_present ? "not present" : "rights violation",
              write ? "writing" : "reading",
              user ? "user" : "kernel");

              printf("Page info:\n Program name: %s\n %s\n %s\n %s\n", spe->t->full_name,
              spe->executable_page == true ? "is executable_page" : "not exectuable_page",
              spe->in_filesys == true ? "in filesys" : spe->in_swap == true ? "in swap" : "in frame",
              spe->access == true ? "accessed before" : "never accessed");
      exit(-1);
    }*/



  /* TODO: now that you have the Supplementale
   * page table entry, you can easily figure out
   * the context of the vaddr, e.g. which virtual page
   * of the process thought it could access this page
   * in physical memory. You can then figure out
   * how to handle this fault, e.g. swap something*/

   // For implementing swap, use:
   // see:  Standford Pintos 4.1.6 Managing Swap Table
   //struct block* swap = block_get_role(BLOCK_SWAP);

   // We know the page that faulted was for code/heap
   if(spe != NULL && spe->executable_page)
   {
     // Need to utilize in_filesys and in_swap when Implementing
     // eviction! OR ELSE assertion will FAIL
     ASSERT(spe->in_filesys != false || spe->in_swap != false);

     // Try to get a frame for the page that isn't in physical memory, which
     // thus is the reason why there was a page fault
     uint8_t* kp = frame_request(spe);

     // Load up the required code, here we know there is a frame we can use
     // for loading the code in, and thus making the page no longer fault!
     if(spe->in_filesys == true && kp != NULL)
     {
       ASSERT(spe->exec_fp != NULL);
       ASSERT(spe->page_read_bytes != -1);
       ASSERT(spe->exec_ofs != -1);
       lock_acquire(&read_write_lock);
       ASSERT(file_read_at (spe->exec_fp, kp, spe->page_read_bytes, spe->exec_ofs) == (int) spe->page_read_bytes);
       lock_release(&read_write_lock);
       memset (kp + spe->page_read_bytes, 0, spe->page_zero_bytes);
       // Install the page to the page directory only if it was missing (not_present == TRUE),
       // if its missing it means the page faulted because the page for the code
       // has not been loaded up yet, it did not fault because the page was swapped
       // out, which would mean install_page has already been done!
       if(not_present) ASSERT(install_page(spe->vaddr, kp, spe->writable));
       return;
     }
     // TODO: IMPLEMENT EVICTION! All frames are used up! Need to evict one!
     else if(spe->in_filesys == true && kp == NULL)
     {
       // This may or may not need to be here
       ASSERT(spe->sector == -1);
       // This needs to be done at the end when eviction is implemented: See code block above
       // if(not_present) ASSERT(install_page(spe->vaddr, kp, spe->writable));
       // return;
     }


   }

   // We know the page that faulted is for a stack
   else {
     printf("stacktime\n");
     // This may or may not need to be here
     ASSERT(spe->sector != -1);
     // You need to utilize the in_swap variable when impelemnting eviction/swap for stack
     ASSERT(spe->in_swap != false);
     // TODO: Handle a swap for stack
   }


  /* To implement virtual memory, delete the rest of the function
     body, and replace it with code that brings in the page to
     which fault_addr refers. */
     // We won't delete this, since when we see this print we will know
     // there is something we haven't added into this function
     // that needs to be added




  printf ("Page fault at %p: %s error %s page in %s context.\n Page Info:\n",
          fault_addr,
          not_present ? "not present" : "rights violation",
          write ? "writing" : "reading",
          user ? "user" : "kernel");


  kill(f, spe);
}
