#include "threads/palloc.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/synch.h"
#include "userprog/process.h"
#include "userprog/pagedir.h"
#include "vm/frame.h"

struct list frame_table;
struct lock frame_table_lock;

// Intiialize any variables or data structures used for frame tables
// Called in init.c like the others
void frame_intitialization()
{
  list_init(&frame_table);
  lock_init(&frame_table_lock);
}

// Insert a new element into the frame table
void* frame_table_insert(struct supplement_page_table_elem* spe, void* kp)
{
  // TODO: Check if already inserted
  struct frame_table_element* fte = malloc(sizeof(struct frame_table_element));
  fte->spe = spe;
  fte->kpe = kp;
  list_push_back(&frame_table,&fte->elem_frame);
}

// Add a frame that is requested by a new virtual page allocation
void* frame_request(struct supplement_page_table_elem* spe){
  lock_acquire(&frame_table_lock);
  void* frame = palloc_get_page(PAL_USER);
  if(frame != NULL)
  {
    frame_table_insert(spe, frame);
    lock_release(&frame_table_lock);
    return frame;
  }
  else {
    lock_release(&frame_table_lock);
    return NULL;
  }
}
