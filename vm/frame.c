#include "threads/palloc.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/synch.h"
#include "userprog/process.h"
#include "userprog/pagedir.h"
#include "vm/frame.h"

struct hash frame_table;
struct lock frame_table_lock;

// Function that creates a hash value for a given frame table entry
unsigned frame_hash_func (const struct hash_elem *e, void *aux UNUSED)
{
  struct frame_table_element * fte = hash_entry(e,struct frame_table_element,
                                                  elem_frame);
  ASSERT(fte->kpe != NULL);
  return (unsigned) fte->kpe;
}

/* Compares the value of two hash elements A and B, given
   auxiliary data AUX.  Returns true if A is less than B, or
   false if A is greater than or equal to B. */
// TODO: May be helpful when doing TLB
bool frame_less_func (const struct hash_elem *a,
                                const struct hash_elem *b,
                                void *aux UNUSED)
  {
    struct frame_table_element * fteA = hash_entry(a,struct frame_table_element,
                                                    elem_frame);
    struct frame_table_element * fteB = hash_entry(b,struct frame_table_element,
                                                    elem_frame);
    if(fteA->kpe < fteB->kpe)
      return true;
    else
      return false;
  }

  // Insert a new element into the frame table
  void* frame_table_insert(struct supplement_page_table_elem* spe, uint8_t* kp)
  {
    // TODO: Check if already inserted
    struct frame_table_element* fte = malloc(sizeof(struct frame_table_element));
    ASSERT(fte != NULL);
    fte->spe = spe;
    fte->kpe = kp;
    if(spe!=NULL)
    {
      if(spe -> executable_page == true)
        fte->spe->in_filesys = false;
      fte->spe->access = true;
    }
    lock_acquire(&frame_table_lock);
    hash_insert(&frame_table,&fte->elem_frame);
    lock_release(&frame_table_lock);
    return fte;

  }

// Intiialize any variables or data structures used for frame tables
// Called in init.c like the others
void frame_intitialization()
{
  hash_init(&frame_table, frame_hash_func, frame_less_func, NULL);
  lock_init(&frame_table_lock);

  uint8_t* kp = palloc_get_page(PAL_USER | PAL_ZERO);
  ASSERT(kp != NULL);
  do{
    frame_table_insert(NULL, kp);
    kp = palloc_get_page(PAL_USER | PAL_ZERO);
  } while(kp != NULL);

}


// return an unused frame's address that is requested by a virtual page
uint8_t* frame_request(struct supplement_page_table_elem* spe){
  /* This implementation is commented out as a backup
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
  } */
    lock_acquire(&frame_table_lock);
    // search through the fd_table for unused frame table entry
    struct hash_iterator i;
    hash_first(&i, &frame_table);
    while(hash_next(&i))
    {
      struct frame_table_element * fte = hash_entry(hash_cur (&i),
                                struct  frame_table_element, elem_frame);
      ASSERT(fte->kpe != NULL);
      if(fte->spe == NULL)
      {
        fte->spe = spe;
        lock_release(&frame_table_lock);
        return fte->kpe;
      }
    }
    lock_release(&frame_table_lock);
    return NULL;
}


// Returns the frame table element associated with
// a given physical frame address
struct frame_table_elem * frame_find(const void* kpe)
{
  lock_acquire(&frame_table_lock);

  struct frame_table_element fte;
  fte.kpe = kpe;
  struct hash_elem * e = hash_find(&frame_table, &fte);
  if(e == NULL) {lock_release(&frame_table_lock); return NULL;}
  struct frame_table_element * fte_found = hash_entry(e,
                            struct  frame_table_element, elem_frame);
 lock_release(&frame_table_lock);
  return fte_found;
}

// Frees up a frame that is being used
bool frame_free(const void* kpe)
{
  struct frame_table_element* fte = frame_find(kpe);
  if(fte == NULL) { return false;}
  lock_acquire(&frame_table_lock);
  fte->spe = NULL;
  lock_release(&frame_table_lock);
  return true;

}

// TODO: Write Frame Table Destructor
