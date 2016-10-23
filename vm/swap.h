#ifndef VM_SWAP_H
#define VM_SWAP_H
#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "devices/shutdown.h"
#include "userprog/process.h"
#include "filesys/filesys.h"
#include "threads/synch.h"
#include "vm/page.h"
#include "vm/frame.h"
#include "kernel/hash.h"
void swap_init();
uint8_t* swap_frame(struct frame_table_element* fte,
          struct supplement_page_table_elem* new_page
        );

#endif
