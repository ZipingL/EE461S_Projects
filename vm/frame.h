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
#include "kernel/hash.h"

struct frame_table_element {
  struct hash_elem elem_frame;
  uint8_t* kpe; // Contains the addres in which the page
  struct supplement_page_table_elem* spe; // See process.h
};

uint8_t* frame_request(struct supplement_page_table_elem* spe);
struct frame_table_elem * frame_find(const void* kpe);
bool frame_free(const void* kpe);

bool frame_free_nolock(const void* kpe);

#endif
