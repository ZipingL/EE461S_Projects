#include "devices/timer.h"
#include <debug.h>
#include <inttypes.h>
#include <round.h>
#include <stdio.h>
#include "devices/pit.h"
#include "threads/interrupt.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "threads/priority.h"
  
/* See [8254] for hardware details of the 8254 timer chip. */

#if TIMER_FREQ < 19
#error 8254 timer requires TIMER_FREQ >= 19
#endif
#if TIMER_FREQ > 1000
#error TIMER_FREQ <= 1000 recommended
#endif

/* Number of timer ticks since OS booted. */
static int64_t ticks;

/* List of processes in THREAD_BLOCKED state, that is, processes
   that are sleeping and will need to eventually move to the ready list. */
static struct list sleep_list;

/* Number of loops per timer tick.
   Initialized by timer_calibrate(). */
static unsigned loops_per_tick;

static intr_handler_func timer_interrupt;
static bool too_many_loops (unsigned loops);
static void busy_wait (int64_t loops);
static void real_time_sleep (int64_t num, int32_t denom);
static void real_time_delay (int64_t num, int32_t denom);


/* Sets up the timer to interrupt TIMER_FREQ times per second,
   and registers the corresponding interrupt. */
void
timer_init (void) 
{
  pit_configure_channel (0, 2, TIMER_FREQ);
  intr_register_ext (0x20, timer_interrupt, "8254 Timer");
  list_init (&sleep_list); //List of threads in THREAD_BLOCKED state (i.e. sleeping threads)
}

/* Calibrates loops_per_tick, used to implement brief delays. */
void
timer_calibrate (void) 
{
  unsigned high_bit, test_bit;

  ASSERT (intr_get_level () == INTR_ON);
  printf ("Calibrating timer...  ");

  /* Approximate loops_per_tick as the largest power-of-two
     still less than one timer tick. */
  loops_per_tick = 1u << 10;
  while (!too_many_loops (loops_per_tick << 1)) 
    {
      loops_per_tick <<= 1;
      ASSERT (loops_per_tick != 0);
    }

  /* Refine the next 8 bits of loops_per_tick. */
  high_bit = loops_per_tick;
  for (test_bit = high_bit >> 1; test_bit != high_bit >> 10; test_bit >>= 1)
    if (!too_many_loops (high_bit | test_bit))
      loops_per_tick |= test_bit;

  printf ("%'"PRIu64" loops/s.\n", (uint64_t) loops_per_tick * TIMER_FREQ);
}

/* Returns the number of timer ticks since the OS booted. */
int64_t
timer_ticks (void) 
{
  enum intr_level old_level = intr_disable ();
  int64_t t = ticks;
  intr_set_level (old_level);
  return t;
}

/* Returns the number of timer ticks elapsed since THEN, which
   should be a value once returned by timer_ticks(). */
int64_t
timer_elapsed (int64_t then) 
{
  return timer_ticks () - then;
}

/* Sleeps for approximately TICKS timer ticks.  Interrupts must
   be turned on. */
void
timer_sleep (int64_t ticks) 
{
  enum intr_level old_level = intr_disable(); //Disable all interrupts (for synchronization)
  thread_current()->tick_cutoff = timer_ticks() + ticks; //Let the thread know when it is supposed to wake up

  if (!list_size(&sleep_list)) { //If there are no elements as of yet, a special exception needs to be made
	list_insert(list_tail(&sleep_list), &thread_current()->elem);
  }
  else { //Otherwise, add the element normally
	list_push_back(&sleep_list, &thread_current()->elem); //This moves the thread from the ready list to the sleep list
  }
  thread_block(); //Move the thread to the blocked state (this is also why the interrupts were disabled)
  intr_set_level(old_level); //Now we need to re-enable interrupts

  //struct sleeping_thread* st = malloc(sizeof(struct sleeping_thread)); //Will point to the node to add to the sleeping thread list
  
  /* Old implementation 
  struct list_elem* entry = list_remove(&thread_current()->elem); //Removes the element associated with the current thread from the ready list
  struct thread* thread_to_stop = list_entry(&thread_current()->elem, struct thread, elem); //Now we get the thread itself
  st->t = thread_to_stop; //Assign the pointer to the current thread
  st->tick_cutoff = timer_ticks() + ticks; //The cutoff for when the thread needs to go back to the ready list
  list_push_front(&sleep_list, &st->elem); //Add the node to the sleep list
  thread_block(); //Now put the current thread to sleep
 // int64_t start = timer_ticks ();

  intr_set_level(old_level); */
  //ASSERT (intr_get_level () == INTR_ON);
  //while (timer_elapsed (start) < ticks) 
    //thread_yield ();
}

/* Sleeps for approximately MS milliseconds.  Interrupts must be
   turned on. */
void
timer_msleep (int64_t ms) 
{
  real_time_sleep (ms, 1000);
}

/* Sleeps for approximately US microseconds.  Interrupts must be
   turned on. */
void
timer_usleep (int64_t us) 
{
  real_time_sleep (us, 1000 * 1000);
}

/* Sleeps for approximately NS nanoseconds.  Interrupts must be
   turned on. */
void
timer_nsleep (int64_t ns) 
{
  real_time_sleep (ns, 1000 * 1000 * 1000);
}

/* Busy-waits for approximately MS milliseconds.  Interrupts need
   not be turned on.

   Busy waiting wastes CPU cycles, and busy waiting with
   interrupts off for the interval between timer ticks or longer
   will cause timer ticks to be lost.  Thus, use timer_msleep()
   instead if interrupts are enabled. */
void
timer_mdelay (int64_t ms) 
{
  real_time_delay (ms, 1000);
}

/* Sleeps for approximately US microseconds.  Interrupts need not
   be turned on.

   Busy waiting wastes CPU cycles, and busy waiting with
   interrupts off for the interval between timer ticks or longer
   will cause timer ticks to be lost.  Thus, use timer_usleep()
   instead if interrupts are enabled. */
void
timer_udelay (int64_t us) 
{
  real_time_delay (us, 1000 * 1000);
}

/* Sleeps execution for approximately NS nanoseconds.  Interrupts
   need not be turned on.

   Busy waiting wastes CPU cycles, and busy waiting with
   interrupts off for the interval between timer ticks or longer
   will cause timer ticks to be lost.  Thus, use timer_nsleep()
   instead if interrupts are enabled.*/
void
timer_ndelay (int64_t ns) 
{
  real_time_delay (ns, 1000 * 1000 * 1000);
}

/* Prints timer statistics. */
void
timer_print_stats (void) 
{
  printf ("Timer: %"PRId64" ticks\n", timer_ticks ());
}

/* Timer interrupt handler. */
static void
timer_interrupt (struct intr_frame *args UNUSED)
{
  ticks++; //Increment # of ticks since the OS booted
  thread_tick (); //Used to update stats of the thread itself

  struct list_elem* e = list_begin(&sleep_list); //Get the element at the head of the list
  while (e != list_end(&sleep_list) && list_size(&sleep_list) > 0) { //Go through the list of sleeping threads, if any.
	struct thread *st = list_entry(e, struct thread, elem); //Get the list entry itself
	if (ticks >= st->tick_cutoff) { //If the tick cutoff for the thread has passed the # of ticks since the OS booted
	  list_remove(e); //Take the thread off the sleep list
	  thread_unblock(st); //Add the thread back to the ready list
	  e = list_begin(&sleep_list); //Now set e back to the start of the sleep list
	  continue; //This iteration is done. Now go to the next thread to check in the sleep list (the new head of sleep list)
	}
	e = e->next; //You have to go through every guy in the list to see if ANY thread needs to wake up
	//e = list_next(e);
  }

/* Old implementation
  for (struct list_elem* e = list_begin(&sleep_list); e != list_end (&sleep_list); e = list_next(e)) { //Go through the sleeping threads list to see if any of the threads are sleeping
	struct sleeping_thread *st = list_entry(e, struct sleeping_thread, elem); //Get the list entry itself
	if (ticks >= st->tick_cutoff) { //If the tick cutoff for the thread has passed the # of ticks since the OS booted
	  struct thread *t = st->t; //Just to be safe, go ahead and get the data of the sleeping thread
	  list_remove(e); //Take the thread off the sleep list
	  thread_unblock(t); //Add the thread back to the ready list
	}
  } */
}

/* Returns true if LOOPS iterations waits for more than one timer
   tick, otherwise false. */
static bool
too_many_loops (unsigned loops) 
{
  /* Wait for a timer tick. */
  int64_t start = ticks;
  while (ticks == start)
    barrier ();

  /* Run LOOPS loops. */
  start = ticks;
  busy_wait (loops);

  /* If the tick count changed, we iterated too long. */
  barrier ();
  return start != ticks;
}

/* Iterates through a simple loop LOOPS times, for implementing
   brief delays.

   Marked NO_INLINE because code alignment can significantly
   affect timings, so that if this function was inlined
   differently in different places the results would be difficult
   to predict. */
static void NO_INLINE
busy_wait (int64_t loops) 
{
  while (loops-- > 0)
    barrier ();
}

/* Sleep for approximately NUM/DENOM seconds. */
static void
real_time_sleep (int64_t num, int32_t denom) 
{
  /* Convert NUM/DENOM seconds into timer ticks, rounding down.
          
        (NUM / DENOM) s          
     ---------------------- = NUM * TIMER_FREQ / DENOM ticks. 
     1 s / TIMER_FREQ ticks
  */
  int64_t ticks = num * TIMER_FREQ / denom;

  ASSERT (intr_get_level () == INTR_ON);
  if (ticks > 0)
    {
      /* We're waiting for at least one full timer tick.  Use
         timer_sleep() because it will yield the CPU to other
         processes. */                
      timer_sleep (ticks); 
    }
  else 
    {
      /* Otherwise, use a busy-wait loop for more accurate
         sub-tick timing. */
      real_time_delay (num, denom); 
    }
}

/* Busy-wait for approximately NUM/DENOM seconds. */
static void
real_time_delay (int64_t num, int32_t denom)
{
  /* Scale the numerator and denominator down by 1000 to avoid
     the possibility of overflow. */
  ASSERT (denom % 1000 == 0);
  busy_wait (loops_per_tick * num / 1000 * TIMER_FREQ / (denom / 1000)); 
}
