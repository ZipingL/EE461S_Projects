#ifndef VM_PAGE_H
#define VM_PAGE_H
#include "devices/block.h"
#include "filesys/off_t.h"
// Struct to be used for supplmenetal page table (SPT) list
struct supplement_page_table_elem {
	struct list_elem spt_elem; // needed by linked list
	uint8_t* vaddr; // holds the virtual address that the page is associated with
	 // determines if the data in the page holds executable code
	 // false means its stack
	 // true means code page or heap
	bool executable_page;
	bool in_swap; // True if page was evicted into swap area
	bool pin; // Indicates if the page shouldn't be evicted
	bool access; // Indicates if the page has been accessed (put in a frame before)
	int sector; // Indicates where in the swap area the page is located
	bool in_filesys; // True if executable page was evicted
	int page_read_bytes; // Contains number of bytes a page holds of exec file
	int page_zero_bytes;
	off_t exec_ofs;
	bool writable;
	struct file * exec_fp; // Holds the executable file, set in page.c->page_find_spe()
	struct thread * t; // Holds the thread that owns the virtual page
	uint8_t* kpe; // Holds the kernel vritual addr, or frame addr
};

struct supplement_page_table_elem* page_find_spe(
  void* virtual_address);

struct supplement_page_table_elem* page_add_supplemental_elem
      (struct list* spt, struct thread* t,
      uint8_t* vaddr, bool executable_page);

bool page_supplemental_entry_remove(void* uva);
void unpin_pointer(uint8_t* pointer);


#endif
