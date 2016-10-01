#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H
#include "threads/thread.h"
#define MAX_ARGS_COUNT 100
#define MAX_FILE_NAME_SIZE 20

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (int exit_status);
void process_activate (void);

// We need to access a void* pointer in two different ways
// A union will help faciliate that
typedef union {
	uint32_t* p_word;
	char* p_byte;
} unioned_esp_pointer_t;

// This is for our linked list of fds
struct fd_list_element {
	struct list_elem elem_fd;
	int fd;
	struct file* fp;
};

// enums to be used for the child status
enum child_status_t {
  PROCESS_RUNNING,
  PROCESS_STOPPED,
  PROCESS_DONE
};

// Struct to be used for the child list
struct child_list_elem {
  	struct list_elem elem_child; // use the linked list data type proved by pintos in list.h/list.c
	tid_t pid;
	tid_t parent_pid;
	enum child_status_t status; //The status of the child as an enum, no need for a zombie state
	struct semaphore* sema;
	int exit_status ;
};


#endif /* userprog/process.h */
