#include "threads/thread.h"
#include <list.h>
#include <debug.h>
#include <stddef.h>
#include <random.h>
#include <stdio.h>
#include <string.h>

/* List of processes in THREAD_BLOCKED state, that is, processes
   that are sleeping and will need to eventually move to the ready list. */
struct list sleep_list;

struct sleeping_thread {
	struct thread* t; //The thread that is sleeping right now
	struct list_elem elem; //The thing that makes the list stuff work
	int64_t tick_cutoff; //The number of milliseconds that the thread will need to wait before being called to run
};
