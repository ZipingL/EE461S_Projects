#include "threads/thread.h"
#include <list.h>
#include <debug.h>
#include <stddef.h>
#include <random.h>
#include <stdio.h>
#include <string.h>

struct sleeping_thread {
	struct thread* t; //The thread that is sleeping right now
	struct list_elem elem; //The thing that makes the list stuff work
	int64_t tick_cutoff; //The number of milliseconds that the thread will need to wait before being called to run
};

void sort_ready_list(struct list* list);
