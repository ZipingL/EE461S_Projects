#ifndef VM_FRAME_H
#define VM_FRAME_H

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

struct frame_table_element {
  struct list_elem elem_frame;
  void* kpe; // Contains where the physical memory of frame is.,
  struct supplement_page_table_elem* spe; // See process.h
};

void* frame_request(struct supplement_page_table_elem* spe);


#endif
