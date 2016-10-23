#include "threads/palloc.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/synch.h"
#include "userprog/process.h"
#include "userprog/pagedir.h"
#include "vm/swap.h"

#include "vm/frame.h"
#include "devices/block.h"

struct bitmap * swap_table = NULL; // element represents a page in the swap block
struct lock swap_lock;
void swap_init()
{
  struct block* swap_table = block_get_role (BLOCK_SWAP);
  ASSERT(swap_table != NULL);
  swap_table = bitmap_create( block_size(swap_table) / SECTORS_PER_PAGE);
  ASSERT(swap_table != NULL);
  bitmap_set_all(swap_table, true); // make all swap areas free (true == free)
  lock_init(&swap_lock);
}

uint8_t* swap_frame(struct frame_table_element* fte,
          struct supplement_page_table_elem* new_page
          )
{
  lock_acquire(&swap_lock);
  struct block* swap = block_get_role (BLOCK_SWAP);

  if(fte->spe->executable_page == false)
  {
    int free_swap_area = bitmap_scan_and_flip(swap_table, 0, 1, true);
    for (int i = 0; i < SECTORS_PER_PAGE; i++)
    {
      block_write(swap, free_swap_area*SECTORS_PER_PAGE + i,
        (uint8_t*) fte->kpe + i*BLOCK_SECTOR_SIZE);
    }

    fte->spe->sector = free_swap_area;
    ASSERT(fte->spe->in_swap == false);
    fte->spe->in_swap = true;
  }
  else {
    //ASSERT(fte->spe->in_filesys == false);

    // Looks like this wasn't a missing page at all, but an invalid access
    // that the user program did!
    if(fte->spe->in_filesys == false)
      exit(-1);
    fte->spe->in_filesys = true;
  }

  // Makesure the page direcotry knows that page is swapped
  pagedir_clear_page (fte->spe->t->pagedir, fte->spe->vaddr);
  // put in the new page into frame
  fte->spe = new_page;
  fte->spe->kpe = fte->kpe;

  // write into the frame, what the swap had if necessary
  // if its an exectuable page, we do it in the page fault handler
  if(new_page->in_swap == true)
  {
    for (int i = 0; i < SECTORS_PER_PAGE; i++)
    {
      block_read(swap, new_page->sector*SECTORS_PER_PAGE + i,
        (uint8_t*) fte->kpe + i*BLOCK_SECTOR_SIZE);
    }
  }

  // Now return the frame address
  // In case the frame writing is done in page fault handler
  lock_release(&swap_lock);
  return fte->kpe;
}
