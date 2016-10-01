#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "devices/shutdown.h"
#include "userprog/process.h"
#include "filesys/filesys.h"



struct child_list_elem* add_child_to_list(struct thread* parent_thread, tid_t pid);

static int
get_user (const uint8_t *uaddr);
static void syscall_handler (struct intr_frame *);
struct list_elem* find_fd_element(int fd, struct thread* current_thread);
bool create (const char *file, unsigned initial_size);
int open (const char *file);

bool close (int fd);

void exit (int status, struct intr_frame *f);
int write (int fd, const void *buffer, unsigned size);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) //UNUSED) 
{
	//Check for valid esp
	if(get_user(f->esp) == -1)
		exit(-1, f);

	//Assume that the esp pointer goes to the top of the stack (looks at return address)
	uint32_t system_call_number =* (uint32_t**)(f->esp+0); //Create a pointer to the top of the stack (looks at argv[0])
	uint32_t* stack_ptr =  (uint32_t*)(f->esp+0); // Two pointers with same address, but using different names
	// to avoid confusion in usage
	char* name = NULL;
	uint32_t file_size = 0;
	int fd = -1;
/*
	if (pid_t == 0) { //The process id is 0 if the process is a child
		struct child_list *list = struct child_list; //Point to the existing list of children
		while (list->next != NULL) { //Iterate through the list
			list = list->next;
		}
		struct child_list *newChild = malloc(struct child_list);
		list->next = newChild; //Link the list
		newChild->pid = find_thread(pid_t)->tid; //Get the thread id of the current process
		newChild->status = RUNNING; //Since you know that the process is running
		newChild->next = NULL;
	}*/

	switch(system_call_number) { //This gives us the command that needs to be executed
		case SYS_CREATE: //A pre-defined constant that refers to a "create" call
		{
			name = *(stack_ptr+1); //With this, we can load the name of the file
			if (name == NULL || *name == NULL) {
				exit(-1, f); //If the pointer or file name is empty, then return an error code
			}
			file_size = *(stack_ptr+2); //Now get the second arg: the size of the file
			f->eax = create(name, file_size); //Create the file and then save the status to the eax register
			break;
		}
			//(Does this mean that eax is just some storage register. What is it really??)
		case SYS_OPEN: //A pre-defined constant that refers to an "open" call
		{
			name = *(stack_ptr+1); //This looks just to the first and only needed parameter, the file to open
			if (name == NULL || *name == NULL) { //Check for a non-existant file of course
				exit(-1, f);
			}
			f->eax = open(name); //Going to refer from eax from now on as the "status" register
			break;
		}
		case SYS_CLOSE:
		{
			fd = *(stack_ptr+1); //Just do something almost exactly the same as what was done for SYS_CREATE
			if (fd <= 0) {
				exit(-1, f); //If the pointer or file name is empty, then return an error code
			}
			file_size = *(stack_ptr+2);
			f->eax = close(fd); //The only line different from SYS_OPEN
			break;
		}
		case SYS_READ:
		{

			fd = *(stack_ptr+1);
			void* buffer = *(stack_ptr+2);
			//Check for valid buffer
			if(get_user(buffer) == -1)
				exit(-1, f);


			file_size = *(stack_ptr+3);
			f->eax = read(fd, buffer, file_size);
			break;
		}
		case SYS_WRITE:
		{
			fd = *(stack_ptr+1);
			void* buffer = *(stack_ptr+2);
			//Check for valid buffer
			if(get_user(buffer) == -1)
					exit(-1, f);
			file_size = *(stack_ptr+3);

			f->eax = write(fd, buffer, file_size);

			break;
		}

		case SYS_FILESIZE:
		{
			fd = *(stack_ptr + 1);
			f->eax = filesize_get(fd);
			break;
		}

		case SYS_EXIT:
	      {
			fd = *(stack_ptr+1);
			exit(fd, f);
			break;
	      }

	    case SYS_EXEC:
	    {
	    	name = *(stack_ptr+1);
	    	f->eax = exec(name);
	    	break;
	    }

	    case SYS_WAIT:
	    {
	    	fd = *(stack_ptr + 1);
	    	f->eax = wait(fd);
	    	break;
	    }

		default:
		{
			//#ifdef PROJECT2_DEBUG
			printf("DID NOT IMPLEMENT THIS SYSCALL ERROR, number:%d\n", system_call_number);
			//#endif
			break;
		}
		}

}


tid_t exec(const char* name)
{
	tid_t pid = process_execute(name);

	if(pid != TID_ERROR)
		return pid;
	else
		return -1;
}

/* Used by syscall_filesize*/
int filesize_get(int fd)
{
	struct thread* current_thread = thread_current();
	struct list_elem* e = find_fd_element(fd, current_thread);
	if(e == NULL) return -1; // return false if fd not found TODO?
	struct  fd_list_element *fd_element = list_entry (e, struct fd_list_element, elem_fd);
	return file_length (fd_element -> fp) ;
}

/* Terminates Pintos by calling shutdown_power_off() (declared in "threads/init.h"). This should be seldom used, because you lose some information about possible deadlock situations, etc. */

void halt (void) {
	shutdown_power_off(); //Fairly straightforward
}
    
/* Terminates the current user program, returning status to the kernel. If the process's parent waits for it (see below), this is the status that will be returned. Conventionally, a status of 0 indicates success and nonzero values indicate errors. */

void exit (int status, struct intr_frame *f) {


	f->eax = status; //Save the status that was returned by the existing process to the stack
	struct thread* t = thread_current();
	printf ("%s: exit(%d)\n", t->name, status);


	thread_exit_process(status); //A function in thread.h that terminates and removes from the list of threads the current thread t. t's status also becomes THREAD_DYING
}



 /* Waits for a child process pid and retrieves the child's exit status.

    If pid is still alive, waits until it terminates. Then, returns the status that pid passed to exit. If pid did not call exit(), but was terminated by the kernel (e.g. killed due to an exception), wait(pid) must return -1. It is perfectly legal for a parent process to wait for child processes that have already terminated by the time the parent calls wait, but the kernel must still allow the parent to retrieve its child's exit status, or learn that the child was terminated by the kernel. wait must fail and return -1 immediately if any of the following conditions is true:
        - pid does not refer to a direct child of the calling process. pid is a direct child of the calling process if and only if the calling process received pid as a return value from a successful call to exec.
        - Note that children are not inherited: if A spawns child B and B spawns child process C, then A cannot wait for C, even if B is dead. A call to wait(C) by process A must fail. Similarly, orphaned processes are not assigned to a new parent if their parent process exits before they do.
		- The process that calls wait has already called wait on pid. That is, a process may wait for any given child at most once. 
		- Processes may spawn any number of children, wait for them in any order, and may even exit without having waited for some or all of their children. Your design should consider all the ways in which waits can occur. All of a process's resources, including its struct thread, must be freed whether its parent ever waits for it or not, and regardless of whether the child exits before or after its parent.

    You must ensure that Pintos does not terminate until the initial process exits. The supplied Pintos code tries to do this by calling process_wait() (in "userprog/process.c") from main() (in "threads/init.c"). We suggest that you implement process_wait() according to the comment at the top of the function and then implement the wait system call in terms of process_wait().

    Implementing this system call requires considerably more work than any of the rest. */

int wait (tid_t pid) {
	return process_wait(pid);
}

/* Creates a new file called file initially initial_size bytes in size. Returns true if successful, false otherwise. Creating a new file does not open it: opening the new file is a separate operation which would require a open system call. */

bool create (const char *file, unsigned initial_size) {
	bool return_bool = filesys_create(file, initial_size); //Already in filesys.c...

	return return_bool;
}

/* Deletes the file called file. Returns true if successful, false otherwise. A file may be removed regardless of whether it is open or closed, and removing an open file does not close it. See Removing an Open File, for details. */
//TODO
//bool remove (const char *file)

/* Opens the file called file. Returns a nonnegative integer handle called a "file descriptor" (fd), or -1 if the file could not be opened.

    File descriptors numbered 0 and 1 are reserved for the console: fd 0 (STDIN_FILENO) is standard input, fd 1 (STDOUT_FILENO) is standard output. The open system call will never return either of these file descriptors, which are valid as system call arguments only as explicitly described below.

    Each process has an independent set of file descriptors. File descriptors are not inherited by child processes.

    When a single file is opened more than once, whether by a single process or different processes, each open returns a new file descriptor. Different file descriptors for a single file are closed independently in separate calls to close and they do not share a file position. */

int open (const char *file) {

	struct thread* current_thread = thread_current();
	struct file* fp = filesys_open(file); //Again, already in filesys.c
	int return_fd = -1;
	/* Now update the file descriptor table */
	if (fp != NULL) {

		return_fd = add_file_to_fd_table(current_thread, fp);
	}
	return return_fd; // IF The file could not be assigned a new file descriptor, then return_fd == -1
}

/* Returns the size, in bytes, of the file open as fd. */

//int filesize (int fd)

/* Reads size bytes from the file open as fd into buffer. Returns the number of bytes actually read (0 at end of file), or -1 if the file could not be read (due to a condition other than end of file). Fd 0 reads from the keyboard using input_getc(). */

int read (int fd, void *buffer, unsigned size)
{


	if(fd == 0)
	{
/*TODO, read from stdin*/
	}

	else if (fd != 1)
	{
		struct thread* t = thread_current();
		struct list_elem* e = find_fd_element(fd, t);
		struct fd_list_element *fd_element = list_entry(e, struct fd_list_element, elem_fd);
		return file_read (fd_element->fp, buffer, size) ;
	}
}

/* Writes size bytes from buffer to the open file fd. Returns the number of bytes actually written, which may be less than size if some bytes could not be written.

    Writing past end-of-file would normally extend the file, but file growth is not implemented by the basic file system. The expected behavior is to write as many bytes as possible up to end-of-file and return the actual number written, or 0 if no bytes could be written at all.

    Fd 1 writes to the console. Your code to write to the console should write all of buffer in one call to putbuf(), at least as long as size is not bigger than a few hundred bytes. (It is reasonable to break up larger buffers.) Otherwise, lines of text output by different processes may end up interleaved on the console, confusing both human readers and our grading scripts. */

int write (int fd, const void *buffer, unsigned size) { //Already done in file.c, but will implement anyway (super confused now)
		
	struct thread* current_thread = thread_current();
	struct file* fp = NULL;
	int return_size = -1;

// TODO: Calling Exit won't work here, requires two arguments, see syscall.c -> exit()
	// Alternatively, you could add the second arg required in the parameter list for write
	// but we may not want to do that, if this error can be managed from the calling function


	/*
	// Why are we doing this?
	if (buffer >= PHYS_BASE) { //Complain about the attemplt to write to the kernel
		exit(-1); //By killing the process
	}
	if (fd == 0) { //Complain about the attempt to write to STDIN
		exit(-1); //Again, the punishment is death
	} */
	if (fd == 1) // 
	{	
		putbuf(buffer, size);
		return_size = size;
	}
	else if (fd != 0 && fd !=1) {
		struct list_elem* e = find_fd_element(fd, current_thread);
		struct  fd_list_element *fd_element = list_entry (e, struct fd_list_element, elem_fd);
		return_size = file_write (fd_element->fp, buffer, size) ;
	}

	return return_size;
}

    /* Changes the next byte to be read or written in open file fd to position, expressed in bytes from the beginning of the file. (Thus, a position of 0 is the file's start.)

    A seek past the current end of a file is not an error. A later read obtains 0 bytes, indicating end of file. A later write extends the file, filling any unwritten gap with zeros. (However, in Pintos files have a fixed length until project 4 is complete, so writes past end of file will return an error.) These semantics are implemented in the file system and do not require any special effort in system call implementation. */

//void seek (int fd, unsigned position)

/* Returns the position of the next byte to be read or written in open file fd, expressed in bytes from the beginning of the file. */

//unsigned tell (int fd)
    
/* Closes file descriptor fd. Exiting or terminating a process implicitly closes all its open file descriptors, as if by calling this function for each one. */

bool close (int fd) {
	struct thread* current_thread = thread_current();
	struct list_elem* e = find_fd_element(fd, current_thread);
	if(e == NULL) return false; // return false if fd not found
	struct list_elem*  return_e = list_remove (e);
	struct  fd_list_element *fd_element = list_entry (e, struct fd_list_element, elem_fd);
	free(fd_element); // Free the element we just removed, please also see open()
	return true;


}

// Find the element in the linkedlist coressponding to the given fd
// Uses: Returns the list_element* type
// 			You can thus then use this returned type to remove element or
//          Edit the contents of the actual element
// You can view an example of these two uses in void close function above
struct list_elem* find_fd_element(int fd, struct thread* current_thread)
{
	    struct list_elem *e;
      	// search through the fd_table for the matching fd 
		for (e = list_begin (&current_thread->fd_table); e != list_end (&current_thread->fd_table);
           e = list_next (e))
        {
          struct  fd_list_element *fd_element = list_entry (e, struct fd_list_element, elem_fd);
          if(fd_element->fd == fd)
          {
          	return e;
          }
        }

        return NULL;
}

struct list_elem* find_child_element(struct thread* current_thread, tid_t pid)
{
	    struct list_elem *e;
      	// search through the fd_table for the matching fd 
		for (e = list_begin (&current_thread->child_list); e != list_end (&current_thread->child_list);
           e = list_next (e))
        {
          struct  child_list_elem *child_element = list_entry (e, struct child_list_elem, elem_child);
          if(child_element->pid == pid)
          {
          	return e;
          }
        }

        return NULL;
}


// Find the element in the linkedlist coressponding to the given pid
// Uses: Returns the list_element* type
// 			You can thus then use this returned type to remove element or
//          Edit the contents of the actual element


// TODO: Should check if file is already opened/added
int add_file_to_fd_table(struct thread* current_thread, struct file* fp)
{
		int return_fd = -1;
		struct fd_list_element* fd_element = malloc(sizeof(struct fd_list_element));
		fd_element->fd = current_thread->fd_table_counter;
		fd_element->fp = fp;
		return_fd = fd_element->fd;

		list_push_back(&current_thread->fd_table, &fd_element->elem_fd);
		current_thread->fd_table_counter++; // increment counter, so we have a new fd to use for the next file
		return return_fd;
}
// TODO: Should check if child is already added 
struct child_list_elem* add_child_to_list(struct thread* parent_thread, tid_t pid)
{
		/* create new child element to push to the parent's child list*/
		struct child_list_elem* child_element = malloc(sizeof(struct child_list_elem));
		child_element->pid = pid;
		child_element->parent_pid = parent_thread->tid;
		child_element->status = PROCESS_RUNNING;
		/* also give the child thread struct itself a ptr to the child_element
		   so that the child can update its status/ and exit status and the parent will see */
		// TODO: This may cause concurrency issues by doing it this way, but we will see....
		struct thread* child_thread = find_thread(pid);
		child_thread->child_data = child_element;
		list_push_back(&parent_thread->child_list, &child_element->elem_child);
		return child_element;
}


 	
/* Reads a byte at user virtual address UADDR.
   UADDR must be below PHYS_BASE.
   Returns the byte value if successful, -1 if a segfault
   occurred. */
static int
get_user (const uint8_t *uaddr)
{
  int result;
  asm ("movl $1f, %0; movzbl %1, %0; 1:"
       : "=&a" (result) : "m" (*uaddr));
  return result;
}
 
/* Writes BYTE to user address UDST.
   UDST must be below PHYS_BASE.
   Returns true if successful, false if a segfault occurred. */
static bool
put_user (uint8_t *udst, uint8_t byte)
{
  int error_code;
  asm ("movl $1f, %0; movb %b2, %1; 1:"
       : "=&a" (error_code), "=m" (*udst) : "q" (byte));
  return error_code != -1;
}
