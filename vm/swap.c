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
struct block *swap_block = NULL;
//bool* swap_table;

void swap_init()
{
  swap_block = block_get_role (BLOCK_SWAP);
  ASSERT(swap_block != NULL);
  swap_table = bitmap_create( block_size(swap_block) / SECTORS_PER_PAGE);
  ASSERT(swap_table != NULL);
  bitmap_set_all(swap_table, true); // make all swap areas free (true == free)
/*  swap_table = malloc(sizeof(bool)*(block_size(swap_block) / SECTORS_PER_PAGE));
  for(int i = 0; i < block_size(swap_table)/ SECTORS_PER_PAGE; i++)
  {
    swap_table[i] = true;
  }*/
  lock_init(&swap_lock);
}
/*
int swap_find_block_and_set()
{
  for(int i = 0; i < block_size(swap_block)/ SECTORS_PER_PAGE; i++)
  {
    if(swap_table[i])
      {
        swap_table[i] = false;
        return i;
      }
  }
  return -1;
}*/

uint8_t* swap_frame(struct frame_table_element* fte,
          struct supplement_page_table_elem* new_page
          )
{
  //printf("hello\n");
  lock_acquire(&swap_lock);
//  printf("hello2\n");


  struct block* swap = swap_block;//block_get_role (BLOCK_SWAP);
//  printf("hello3\n");
//printf("swapping out shit");

  if(fte->spe->executable_page == false)
  {
  //  int free_swap_area = swap_find_block_and_set();
   int free_swap_area = bitmap_scan_and_flip(swap_table, 0, 1, true);
   ASSERT(free_swap_area != SIZE_MAX);
    for (int i = 0; i < SECTORS_PER_PAGE; i++)
    {
      bool lock_status = false;
    if(!lock_held_by_current_thread(&read_write_lock))
     {
        lock_status = true;
      lock_acquire(&read_write_lock);
 }
 block_write(swap_block, free_swap_area*SECTORS_PER_PAGE + i,
        (uint8_t*) fte->kpe + i*BLOCK_SECTOR_SIZE);
       if(lock_status)
        lock_release(&read_write_lock);
    }

    fte->spe->sector = free_swap_area;
    ASSERT(fte->spe->in_swap == false);
    fte->spe->in_swap = true;
  }
  else {
    ASSERT(fte->spe->in_filesys == false);
    ASSERT(fte->spe->pin == false);

    // Looks like this wasn't a missing page at all, but an invalid access
    // that the user program did!
  //  if(fte->spe->in_filesys != false)
  //  {
    //  exit(-1); }
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
  //  printf("swap reader\n");
    ASSERT(bitmap_test(swap_table, new_page->sector) == false);
    for (int i = 0; i < SECTORS_PER_PAGE; i++)
    {
      bool lock_status = false;
    if(!lock_held_by_current_thread(&read_write_lock))
     {
        lock_status = true;
      lock_acquire(&read_write_lock);
 }
      block_read(swap_block, new_page->sector*SECTORS_PER_PAGE + i,
        (uint8_t*) fte->kpe + i*BLOCK_SECTOR_SIZE);
        if(lock_status)
        lock_release(&read_write_lock);
    }


    //swap_table[new_page->sector] = true;
    bitmap_set (swap_table, new_page->sector, true);
    new_page->sector = -1;
  }

  // Now return the frame address
  // In case the frame writing is done in page fault handler
  lock_release(&swap_lock);
  return fte->kpe;
}

void swap_table_set(size_t sector, bool value)
{
  bitmap_set(swap_table, sector, value);
}
