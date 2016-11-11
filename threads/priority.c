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
	struct list_elem* others = list_begin(&list); //This is a pointer that will look at the rest of the values
	struct thread* ct = list_entry(e, struct thread, elem);

	while (others != list_end(&list)) { //Now we compare e with all the other threads in the list
	  if (e == others) { //If e and others point at the same element
		others = others->next; //Go to the next element
		continue; //This iteration is useless
	  }
	  struct thread* ot = list_entry(others, struct thread, elem);
	  if (ct->priority < ot->priority) { //If the current thread's priority is less than the priority it is being checked against
		//Then the current thread should move down the list
		e->next = others->next; //Initialize the current thread's next pointer
		e->prev = others; //Now we point to the element that is before e
		others->next->prev = e; //Make sure the next element after e will point to e
		others->next = e; //Now others should point to the element that is after it - e
		break; //Also, break, as the element is now in the correct position
	  }
	  others = others->next; //Else, continue to iterate through the list
	}

	e = e->next; //Now apply the sort to all the other threads as well
  }
}
