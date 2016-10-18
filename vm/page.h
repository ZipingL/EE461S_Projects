#ifndef VM_PAGE_H
#define VM_PAGE_H

// Struct to be used for supplmenetal page table (SPT) list
struct supplement_page_table_elem {
	struct list_elem spt_elem;
	void* vaddr; // holds the virtual address that the page is associated with
	bool executable_page; // determines if the data in the page holds executable code
	struct thread * t; // Holds the thread that owns the virtual page
};

struct supplement_page_table_elem* page_find_spe(struct thread* current_thread,
  void* virtual_address);

struct supplement_page_table_elem* page_add_supplemental_elem
      (struct list* spt, struct thread* t,
      void* vaddr, bool executable_page);

#endif
