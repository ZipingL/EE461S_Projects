#include "userprog/process.h"
#include <debug.h>
#include <inttypes.h>
#include <round.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "userprog/gdt.h"
#include "userprog/pagedir.h"
#include "userprog/tss.h"
#include "filesys/directory.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "threads/flags.h"
#include "threads/init.h"
#include "threads/interrupt.h"
#include "threads/palloc.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/synch.h"
#include "lib/kernel/list.h"

static thread_func start_process NO_RETURN;
static bool load (const char *cmdline, void (**eip) (void), void **esp);
int parse_command_string(char* command, char* argv[], bool set_first_only);


/* Take in Command String and parse it into words 
   Returns the Number of arguments processed
   Assumes that argv[] is large enough to hold all argument string ptrs
   Messes with the contents of the string command through strtok_r
   parameter bool set_first_only means that if calling function just
   wants the first agrv[0] only, then set_first_only is set to true,
   incase the user wants to just get the command string without the arguments
   */

int parse_command_string(char* command, char* argv[], bool set_first_only)
{


  char* save;
  int i = 0;
  argv[i] = strtok_r(command, " ", &save);
  while(argv[i] != NULL)
  {
    i++;
    argv[i] = strtok_r(NULL, " ", &save);

    // Set just the first argument, useful in some cases
    if(set_first_only)
    {
      break;
    }
  }

  return i; // return arg count
}

/* Starts a new thread running a user program loaded from
   FILENAME.  The new thread may be scheduled (and may even exit)
   before process_execute() returns.  Returns the new process's
   thread id, or TID_ERROR if the thread cannot be created. */
tid_t
process_execute (const char *file_name) 
{
  char *fn_copy;
  tid_t tid;

  /* Make a copy of FILE_NAME.
     Otherwise there's a race between the caller and load(). */
  fn_copy = palloc_get_page (0);
  if (fn_copy == NULL)
    return TID_ERROR;
  strlcpy (fn_copy, file_name, PGSIZE);

  /* get the actual file name, instead of the name + arguments */
  int file_char_length = strlen(file_name) + 1;
  char file_name_no_args[file_char_length];
  strlcpy(file_name_no_args, file_name, file_char_length);
  char* argv[1];
  parse_command_string(file_name_no_args, argv, true);

  /* Create a new thread to execute FILE_NAME. */
  tid = thread_create (argv[0], PRI_DEFAULT, start_process, fn_copy);

  if (tid == TID_ERROR)
    palloc_free_page (fn_copy); 
  else {

    // Add created process to the current thread's child_list
  struct thread* t = thread_current();
  // Using tid_t for child id for now, may need to change
  struct child_list_elem* success = add_child_to_list(t, tid);
  if(success ==NULL)
    return TID_ERROR;




  }
  return tid;



}

/* A thread function that loads a user process and starts it
   running. */
static void
start_process (void *file_name_)
{
  char *file_name = file_name_;
  struct intr_frame if_;
  bool success;

  /* Initialize interrupt frame and load executable. */
  memset (&if_, 0, sizeof if_);
  if_.gs = if_.fs = if_.es = if_.ds = if_.ss = SEL_UDSEG;
  if_.cs = SEL_UCSEG;
  if_.eflags = FLAG_IF | FLAG_MBS;
  success = load (file_name, &if_.eip, &if_.esp);

  /* If load failed, quit. */
  palloc_free_page (file_name);
  if (!success) 
    thread_exit ();

  /* Start the user process by simulating a return from an
     interrupt, implemented by intr_exit (in
     threads/intr-stubs.S).  Because intr_exit takes all of its
     arguments on the stack in the form of a `struct intr_frame',
     we just point the stack pointer (%esp) to our stack frame
     and jump to it. */
  asm volatile ("movl %0, %%esp; jmp intr_exit" : : "g" (&if_) : "memory");
  NOT_REACHED ();
}

/* Waits for thread TID to die and returns its exit status.  If
   it was terminated by the kernel (i.e. killed due to an
   exception), returns -1.  If TID is invalid or if it was not a
   child of the calling process, or if process_wait() has already
   been successfully called for the given TID, returns -1
   immediately, without waiting.



   This function will be implemented in problem 2-2.  For now, it
   does nothing. */

      // TODO: This function is not done, this is not the correct way 
   // it's just a hack that barely works, need to implement
   // parent and child lists
   // A correct process wait involves looking at the current thread's
   // child_list, finding the child from the child_tid, and
   // then wait on it.
   // Right now, we are just looking at a list of all threads
   // and waiting on that, so we have no way of knowing whether or not
   // the child_tid is actually a child of the thread,
   // and we have no way of waiting for the child_tid's children too.
   // We should not be looking at threads, we need to move up an abstraction
   // into process, since using threads to check a process's status
   // can cause issues, for example, we are failing a few tests
   // I think due to the this.
int
process_wait (tid_t child_tid UNUSED) 
{

  //Found a possible way to do this!!
  //Now first we need to understand that every parent process can have multiple children and we have to wait for all of those, yes?
  //However, no process inherently keeps a count of its children
  //But Linux does!! Inside /proc, there are directories for each and every process
  //Now, we can just go through all those directories and compare the child_tid's PPID with a given parent ID to see if its a child
  //If a process matches that pid, then this process is a child of that one! For multiple children, we can just build a linked list to iterate through
  //Here's a rough implementation of what I mean: (It could also utterly suck. This is crazy inefficient after all)
  //We'll also have to likely add the parent_ID as a parameter to pass in
/*
  char* procDirPath = "\proc\""; //This is the directory that the status for all processes are located in
  uint16_t largestID = 0;
  uint16_t processToOpen = 2;
  uint16_t fileOpened = 1;
  size_t size = 40;
  char* ppidLine = NULL;
  char* statusInfo = NULL;
  char* status = NULL;
  char* ppid = NULL;*/

 // struct child_list* head = thread_current()->kid_list; //The head of the child list
  //struct child_list* iterator = head;

  /* while (fileOpened) {
    fileOpened = open(strcat(strcat(procDirPath, ++processToOpen), "status"));
	if (fileOpened = -1) { //If the process does not actually have a file in proc, it isn't a process that exists
	  fileOpened = 0; //So the loop should not continue
	  continue;
	}
	for (int i = 0; i < 6; i++) {
		if (i == 1 && getline(&statusInfo, &size, &(strcat(strcat(procDirPath, processToOpen), "status") != -1) { //This is the line that contains the status of the the process file
		
		} //statusInfo has what we need thanks to getLine :P

		if (i == 4 && getline(&ppidLine, &size, &(strcat(strcat(procDirPath, processToOpen), "status") != -1) { //This is the line that contains the status of the the process file
		
		} //ppidLine has what we need thanks to getLine :P
	}

	for (int j = 0; j < strlen(statusInfo); j++) {
	  if (strcmp(statusInfo[j], "S") == 0 || strcmp(statusInfo[j], "R") == 0 || strcmp(statusInfo[j], "X") == 0 || strcmp(statusInfo[j], "x") == 0 || strcmp(statusInfo[j], "Z") == 0) {
		status = statusInfo[j]; //Now store the status of the current process (if it's one that we care about)
		//Status of S means stopped, R means running, X and x mean the process is dead, and Z means zombie
	  }

	for (int k = 0; k < strlen(ppidLine); k++) {
	  if (strcmp(ppidLine[k], "1") == 0 || strcmp(ppidLine[k], "2") == 0 || strcmp(ppidLine[k], "3") == 0 || strcmp(ppidLine[k], "4") == 0 || strcmp(ppidLine[k], "5") == 0 ||
		  strcmp(ppidLine[k], "0") == 0 || strcmp(ppidLine[k], "6") == 0 || strcmp(ppidLine[k], "7") == 0 || strcmp(ppidLine[k], "8") == 0 || strcmp(ppidLine[k], "9") == 0) { //Check for any possible digit, lol
		ppid = strcat(ppid, ppidLine[k]); //Now store the pid of the current process (if it's one that we care about)
	  }

	if (thread_current()->tid == atoi(ppid)) { //If the child is part of the parent list
	  if (iterator == NULL) {
		struct child = malloc(struct child_list); //Create a new node for the child list
		child->pid = child_tid;
		child->ppid = parent_ID;
	  	child->status = status;
	  	child->next = NULL; //Assign the linked list values appropriately
	 	break; //Get out of here, this is the only node in the list after all
	  }

	  while (iterator->next != NULL) {
		iterator = iterator->next;
	  } //Go through the entire list

	  struct child = malloc(struct child_list); //Create a new node for the child list
	  child->pid = child_tid;
	  child->ppid = parent_ID;
	  child->status = status;
	  child->next = NULL; //Assign the linked list values appropriately
	  iterator->next = child; //Link the actual list
	  iterator = head; //Reset the pointer
	}
  } //At the end of this, the child list should be constructed
*/

  struct thread* current_thread = thread_current();
  struct list_elem* e = find_child_element(current_thread, child_tid);
  if(e == NULL) return -1; // return false if fd not found
  struct  child_list_elem *child_element = list_entry (e, struct child_list_elem, elem_child);
  //printf("child_list wait addr: %p\n", child_element);
  /*while(child_element->status != PROCESS_DONE)
  {
    printf("status: %d", child_element->status);
  }*/
  sema_init(&child_element->sema, 0); 
  sema_down(&child_element->sema);

  int exit_status = child_element->exit_status;

  // Remove done child
  list_remove(e);
  // Free the child memory allocation
  free(child_element);

  //TODO: Remove child from childlist

 // return child_element->exit_status;

/* ziping's bullshit wait*/
  /*
  bool wait = false;
  while(find_thread(child_tid)

   != NULL)
  {
    wait = true;
  } //If you cannot find a child process running

#ifdef PROJECT2_DEBUG
  printf("process_wait bool: %d\n", wait);
#endif
  if(wait)
    return 1; //TODO: Return child exit status
  else
    return -1;*/

    return exit_status;
    
}

/* Free the current process's resources. */
/*TODO*/
void
process_exit (int exit_status)
{
  struct thread *cur = thread_current ();
  // Do not free child_data
  cur->child_data->status = PROCESS_DONE;
  cur->child_data->exit_status = exit_status;
  sema_up(&cur->child_data->sema);
  uint32_t *pd;

  /* Destroy the current process's page directory and switch back
     to the kernel-only page directory. */
  pd = cur->pagedir;
  if (pd != NULL) 
    {
      /* Correct ordering here is crucial.  We must set
         cur->pagedir to NULL before switching page directories,
         so that a timer interrupt can't switch back to the
         process page directory.  We must activate the base page
         directory before destroying the process's page
         directory, or our active page directory will be one
         that's been freed (and cleared). */
      cur->pagedir = NULL;
      pagedir_activate (NULL);
      pagedir_destroy (pd);
    }


}

/* Sets up the CPU for running user code in the current
   thread.
   This function is called on every context switch. */
void
process_activate (void)
{
  struct thread *t = thread_current ();

  /* Activate thread's page tables. */
  pagedir_activate (t->pagedir);

  /* Set thread's kernel stack for use in processing
     interrupts. */
  tss_update ();
}
/* We load ELF binaries.  The following definitions are taken
   from the ELF specification, [ELF1], more-or-less verbatim.  */

/* ELF types.  See [ELF1] 1-2. */
typedef uint32_t Elf32_Word, Elf32_Addr, Elf32_Off;
typedef uint16_t Elf32_Half;

/* For use with ELF types in printf(). */
#define PE32Wx PRIx32   /* Print Elf32_Word in hexadecimal. */
#define PE32Ax PRIx32   /* Print Elf32_Addr in hexadecimal. */
#define PE32Ox PRIx32   /* Print Elf32_Off in hexadecimal. */
#define PE32Hx PRIx16   /* Print Elf32_Half in hexadecimal. */

/* Executable header.  See [ELF1] 1-4 to 1-8.
   This appears at the very beginning of an ELF binary. */
struct Elf32_Ehdr
  {
    unsigned char e_ident[16];
    Elf32_Half    e_type;
    Elf32_Half    e_machine;
    Elf32_Word    e_version;
    Elf32_Addr    e_entry;
    Elf32_Off     e_phoff;
    Elf32_Off     e_shoff;
    Elf32_Word    e_flags;
    Elf32_Half    e_ehsize;
    Elf32_Half    e_phentsize;
    Elf32_Half    e_phnum;
    Elf32_Half    e_shentsize;
    Elf32_Half    e_shnum;
    Elf32_Half    e_shstrndx;
  };

/* Program header.  See [ELF1] 2-2 to 2-4.
   There are e_phnum of these, starting at file offset e_phoff
   (see [ELF1] 1-6). */
struct Elf32_Phdr
  {
    Elf32_Word p_type;
    Elf32_Off  p_offset;
    Elf32_Addr p_vaddr;
    Elf32_Addr p_paddr;
    Elf32_Word p_filesz;
    Elf32_Word p_memsz;
    Elf32_Word p_flags;
    Elf32_Word p_align;
  };

/* Values for p_type.  See [ELF1] 2-3. */
#define PT_NULL    0            /* Ignore. */
#define PT_LOAD    1            /* Loadable segment. */
#define PT_DYNAMIC 2            /* Dynamic linking info. */
#define PT_INTERP  3            /* Name of dynamic loader. */
#define PT_NOTE    4            /* Auxiliary info. */
#define PT_SHLIB   5            /* Reserved. */
#define PT_PHDR    6            /* Program header table. */
#define PT_STACK   0x6474e551   /* Stack segment. */

/* Flags for p_flags.  See [ELF3] 2-3 and 2-4. */
#define PF_X 1          /* Executable. */
#define PF_W 2          /* Writable. */
#define PF_R 4          /* Readable. */

static bool setup_stack (void **esp, char* file_name);
static bool validate_segment (const struct Elf32_Phdr *, struct file *);
static bool load_segment (struct file *file, off_t ofs, uint8_t *upage,
                          uint32_t read_bytes, uint32_t zero_bytes,
                          bool writable);

/* Loads an ELF executable from FILE_NAME into the current thread.
   Stores the executable's entry point into *EIP
   and its initial stack pointer into *ESP.
   Returns true if successful, false otherwise. */
bool
load (const char *file_name, void (**eip) (void), void **esp) 
{

  struct thread *t = thread_current ();
  struct Elf32_Ehdr ehdr;
  struct file *file = NULL;
  off_t file_ofs;
  bool success = false;
  int i;

    /* get the actual file name, instead of the name + arguments */
  int file_char_length = strlen(file_name) + 1;
  char file_name_no_args[file_char_length];
  char file_name_cpy[file_char_length];
  strlcpy(file_name_no_args, file_name, file_char_length);
  char* argv[1];
  parse_command_string(file_name_no_args, argv, true);

  /* Allocate and activate page directory. */
  t->pagedir = pagedir_create ();
  if (t->pagedir == NULL) 
    goto done;
  process_activate ();


  /* Open executable file. */
  file = filesys_open (argv[0]);
  if (file == NULL) 
    {
      printf ("load: %s: open failed\n", argv[0]);
      goto done; 
    }

  /* Read and verify executable header. */
  if (file_read (file, &ehdr, sizeof ehdr) != sizeof ehdr
      || memcmp (ehdr.e_ident, "\177ELF\1\1\1", 7)
      || ehdr.e_type != 2
      || ehdr.e_machine != 3
      || ehdr.e_version != 1
      || ehdr.e_phentsize != sizeof (struct Elf32_Phdr)
      || ehdr.e_phnum > 1024)
    {
      printf ("load: %s: error loading executable\n", argv[0]);
      goto done; 
    }

  /* Read program headers. */
  file_ofs = ehdr.e_phoff;
  for (i = 0; i < ehdr.e_phnum; i++) 
    {
      struct Elf32_Phdr phdr;

      if (file_ofs < 0 || file_ofs > file_length (file))
        goto done;
      file_seek (file, file_ofs);

      if (file_read (file, &phdr, sizeof phdr) != sizeof phdr)
        goto done;
      file_ofs += sizeof phdr;
      switch (phdr.p_type) 
        {
        case PT_NULL:
        case PT_NOTE:
        case PT_PHDR:
        case PT_STACK:
        default:
          /* Ignore this segment. */
          break;
        case PT_DYNAMIC:
        case PT_INTERP:
        case PT_SHLIB:
          goto done;
        case PT_LOAD:
          if (validate_segment (&phdr, file)) 
            {
              bool writable = (phdr.p_flags & PF_W) != 0;
              uint32_t file_page = phdr.p_offset & ~PGMASK;
              uint32_t mem_page = phdr.p_vaddr & ~PGMASK;
              uint32_t page_offset = phdr.p_vaddr & PGMASK;
              uint32_t read_bytes, zero_bytes;
              if (phdr.p_filesz > 0)
                {
                  /* Normal segment.
                     Read initial part from disk and zero the rest. */
                  read_bytes = page_offset + phdr.p_filesz;
                  zero_bytes = (ROUND_UP (page_offset + phdr.p_memsz, PGSIZE)
                                - read_bytes);
                }
              else 
                {
                  /* Entirely zero.
                     Don't read anything from disk. */
                  read_bytes = 0;
                  zero_bytes = ROUND_UP (page_offset + phdr.p_memsz, PGSIZE);
                }
              if (!load_segment (file, file_page, (void *) mem_page,
                                 read_bytes, zero_bytes, writable))
                goto done;
            }
          else
            goto done;
          break;
        }
    }

  /* Set up stack. */ //printf("Set up stack\n");
  file_name_cpy[file_char_length];
  strlcpy(file_name_cpy, file_name, file_char_length);
  bool setup_stack_success = setup_stack(esp, file_name_cpy);
  if (!setup_stack_success)
    goto done;

  /* Start address. */
  *eip = (void (*) (void)) ehdr.e_entry;

  success = true;

 done:
  /* We arrive here whether the load is successful or not. */
  file_close (file);
  return success;
}
/* load() helpers. */

static bool install_page (void *upage, void *kpage, bool writable);

/* Checks whether PHDR describes a valid, loadable segment in
   FILE and returns true if so, false otherwise. */
static bool
validate_segment (const struct Elf32_Phdr *phdr, struct file *file) 
{
  /* p_offset and p_vaddr must have the same page offset. */
  if ((phdr->p_offset & PGMASK) != (phdr->p_vaddr & PGMASK)) 
    return false; 

  /* p_offset must point within FILE. */
  if (phdr->p_offset > (Elf32_Off) file_length (file)) 
    return false;

  /* p_memsz must be at least as big as p_filesz. */
  if (phdr->p_memsz < phdr->p_filesz) 
    return false; 

  /* The segment must not be empty. */
  if (phdr->p_memsz == 0)
    return false;
  
  /* The virtual memory region must both start and end within the
     user address space range. */
  if (!is_user_vaddr ((void *) phdr->p_vaddr))
    return false;
  if (!is_user_vaddr ((void *) (phdr->p_vaddr + phdr->p_memsz)))
    return false;

  /* The region cannot "wrap around" across the kernel virtual
     address space. */
  if (phdr->p_vaddr + phdr->p_memsz < phdr->p_vaddr)
    return false;

  /* Disallow mapping page 0.
     Not only is it a bad idea to map page 0, but if we allowed
     it then user code that passed a null pointer to system calls
     could quite likely panic the kernel by way of null pointer
     assertions in memcpy(), etc. */
  if (phdr->p_vaddr < PGSIZE)
    return false;

  /* It's okay. */
  return true;
}

/* Loads a segment starting at offset OFS in FILE at address
   UPAGE.  In total, READ_BYTES + ZERO_BYTES bytes of virtual
   memory are initialized, as follows:

        - READ_BYTES bytes at UPAGE must be read from FILE
          starting at offset OFS.

        - ZERO_BYTES bytes at UPAGE + READ_BYTES must be zeroed.

   The pages initialized by this function must be writable by the
   user process if WRITABLE is true, read-only otherwise.

   Return true if successful, false if a memory allocation error
   or disk read error occurs. */
static bool
load_segment (struct file *file, off_t ofs, uint8_t *upage,
              uint32_t read_bytes, uint32_t zero_bytes, bool writable) 
{
  ASSERT ((read_bytes + zero_bytes) % PGSIZE == 0);
  ASSERT (pg_ofs (upage) == 0);
  ASSERT (ofs % PGSIZE == 0);

  file_seek (file, ofs);
  while (read_bytes > 0 || zero_bytes > 0) 
    {
      /* Calculate how to fill this page.
         We will read PAGE_READ_BYTES bytes from FILE
         and zero the final PAGE_ZERO_BYTES bytes. */
      size_t page_read_bytes = read_bytes < PGSIZE ? read_bytes : PGSIZE;
      size_t page_zero_bytes = PGSIZE - page_read_bytes;

      /* Get a page of memory. */
      uint8_t *kpage = palloc_get_page (PAL_USER);
      if (kpage == NULL)
        return false;

      /* Load this page. */
      if (file_read (file, kpage, page_read_bytes) != (int) page_read_bytes)
        {
          palloc_free_page (kpage);
          return false; 
        }
      memset (kpage + page_read_bytes, 0, page_zero_bytes);

      /* Add the page to the process's address space. */
      if (!install_page (upage, kpage, writable)) 
        {
          palloc_free_page (kpage);
          return false; 
        }

      /* Advance. */
      read_bytes -= page_read_bytes;
      zero_bytes -= page_zero_bytes;
      upage += PGSIZE;
    }
  return true;
}

/* Create a minimal stack by mapping a zeroed page at the top of
   user virtual memory. */

/*The table below shows the state of an example stack and the relevant registers 
right before the beginning of the user program, assuming PHYS_BASE is 0xC0000000:
        Address   Name            Data        Type
        0xbffffffc  argv[3][...]  "bar\0"     char[4]
        0xbffffff8  argv[2][...]  "foo\0"     char[4]
        0xbffffff5  argv[1][...]  "-l\0"      char[3]
        0xbfffffed  argv[0][...]  "/bin/ls\0" char[8]
        0xbfffffec  word-align    0           uint8_t
        0xbfffffe8  argv[4]       0           char *
        0xbfffffe4  argv[3]       0xbffffffc  char *
        0xbfffffe0  argv[2]       0xbffffff8  char *
        0xbfffffdc  argv[1]       0xbffffff5  char *
        0xbfffffd8  argv[0]       0xbfffffed  char *
        0xbfffffd4  argv          0xbfffffd8  char **
        0xbfffffd0  argc          4            int
        0xbfffffcc  return address  0          void (*) ()*/
static bool
setup_stack (void **esp, char* file_name) 
{
  uint8_t *kpage;
  bool success = false;

  kpage = palloc_get_page (PAL_USER | PAL_ZERO);
  if (kpage != NULL) 
    {
      success = install_page (((uint8_t *) PHYS_BASE) - PGSIZE, kpage, true);
      if (success)
      {
        *esp = PHYS_BASE;
        unioned_esp_pointer_t unioned_esp;
        unioned_esp.p_byte = (char*) *esp;
        char* argv[MAX_ARGS_COUNT];
        int argc = parse_command_string(file_name, argv, false);
        int argc_cpy = argc -1; // make a copy argc so we can use as index
        // for storing ptrs of where we stored each string in esp
        uint32_t* esp_arg_ptrs[MAX_ARGS_COUNT]; //Should this be a pointer to integers or an array of ptrs to integers
        // for keeping track of the total bytes used,
        // so we can add necessary padding if need be
        int total_args_length_count = 0;



        /*Push the arguments onto the stack*/
        while(argc_cpy >= 0)
        {
          int arg_length = strlen(argv[argc_cpy]) + 1;
          total_args_length_count+= arg_length;
          // decrement esp byte pointer enough to store the argument
          unioned_esp.p_byte -= sizeof(char)*arg_length;
          // copy the arg string into the esp pointer location
          memcpy(unioned_esp.p_byte, argv[argc_cpy], arg_length);

          esp_arg_ptrs[argc_cpy] = unioned_esp.p_byte; //This should then be a pointer that is assigned
          argc_cpy--;
        }


        int char_per_word = sizeof(uint32_t) / sizeof(char); // should be four, but why not?

        /* Add some padding if necessary */
        int remainder = total_args_length_count % char_per_word;
        if(remainder != 0)
        {
          unioned_esp.p_byte -= (char_per_word - remainder);
        }
        unioned_esp.p_word--; // set esp pointer to next empty uint32_t element
        /* add null to signify end of argv pointers */
        *unioned_esp.p_word = NULL;         
        unioned_esp.p_word--;
        argc_cpy = argc-1;


        /* Push the addresses of the args */
        while(argc_cpy >= 0)
        {
          *unioned_esp.p_word = esp_arg_ptrs[argc_cpy];
          unioned_esp.p_word--;
          argc_cpy--;
        }
        // add a pointer to where argv starts
        char* temp = unioned_esp.p_word + 1;
        *unioned_esp.p_word = temp;
        unioned_esp.p_word --;
        // add argc data
        *unioned_esp.p_word = argc;
        unioned_esp.p_word--;
        // add fake return address
        *unioned_esp.p_word = NULL;

/*
        printf("%d %d\n", unioned_esp.p_word, *esp);
        printf("%s %s %s\n", esp_arg_ptrs[0], esp_arg_ptrs[1], esp_arg_ptrs[2]);
        printf("%s %s %s \n", *((char**)(*(unioned_esp.p_word + 2))+1), *(unioned_esp.p_word + 4), *(unioned_esp.p_word + 5));
        */
        *esp = unioned_esp.p_word;

      }
      else
        palloc_free_page (kpage);
    }
  return success;
}

/* Adds a mapping from user virtual address UPAGE to kernel
   virtual address KPAGE to the page table.
   If WRITABLE is true, the user process may modify the page;
   otherwise, it is read-only.
   UPAGE must not already be mapped.
   KPAGE should probably be a page obtained from the user pool
   with palloc_get_page().
   Returns true on success, false if UPAGE is already mapped or
   if memory allocation fails. */
static bool
install_page (void *upage, void *kpage, bool writable)
{
  struct thread *t = thread_current ();

  /* Verify that there's not already a page at that virtual
     address, then map our page there. */
  return (pagedir_get_page (t->pagedir, upage) == NULL
          && pagedir_set_page (t->pagedir, upage, kpage, writable));
}

