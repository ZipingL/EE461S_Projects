

#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "devices/shutdown.h"
#include "userprog/process.h"
#include "userprog/pagedir.h"
#include "vm/page.h"

// creates a supplementable page table entry
// inserts data into it from input, e.g. struct thread* t
   // There most likely will bemore parameters for
   // More data
// adds the setup entry into the given list* spt
struct supplement_page_table_elem* page_add_supplemental_elem
      (struct list* spt, struct thread* t,
      void* vaddr, bool executable_page)
{
  struct supplement_page_table_elem* spe = malloc(sizeof(
                    struct supplement_page_table_elem
  ));
  spe->t = t;
  spe->vaddr = vaddr;
  spe->executable_page = executable_page; // probably needs to be changed to enum
  list_push_back(spt, &spe->spt_elem);
  return spe;
}

struct supplement_page_table_elem* page_find_spe(struct thread* current_thread,
  void* virtual_address)
{
  struct list_elem *e;
    // search through the fd_table for the matching fd
for (e = list_begin (&current_thread->spt); e != list_end (&current_thread->spt);
       e = list_next (e))
    {
      struct  supplement_page_table_elem *element = list_entry (e, struct supplement_page_table_elem, spt_elem);
      if(element->vaddr == virtual_address)
      {
        return e;
      }
    }

    return NULL;
}
