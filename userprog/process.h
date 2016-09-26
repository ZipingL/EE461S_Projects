#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"
#define MAX_ARGS_COUNT 100

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
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

#endif /* userprog/process.h */
