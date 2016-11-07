#ifndef "threads/thread.h"
#define "threads/thread.h"
#ifndef <list.h>
#define <list.h>

//The file that will contain any functions we wish to implement for priority scheduling

/* A function that will sort the ready list whenever a thread was added. It currently compares every element with every other element. */
void sort_ready_list(struct list* list) {
  struct list_elem* e = list_begin(&list); //Get the value at the beginning of the list
  if (e == NULL || list_size(&list) < 2) {
	return; //No sorting needs to be done on an empty list or a list of size less than 2
  }
  while (e != list_end(&list)) {
	struct list_elem* others = list_next(e); //This is a pointer that will look at the rest of the values
	struct thread* ct = list_entry(e, struct thread, elem);
	while (others != list_end(&list)) {
	  struct thread* ot = list_entry(others, struct thread, elem);
	  if (ct->priority >= ot->priority) { //If the current thread's priority does not need to be moved down the list
		list_insert(others, e); //Then the current thread should go before the thread with the lower priority
		break; //Also, break, as the element is now in the correct position
	  }
	  others = list_next(others); //Else, continue to iterate through the list
	}
	e = list_next(e); //Now check all the other threads as well
  }
}
