/*
 * Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Thread test code.
 */
#include <types.h>
#include <lib.h>
#include <thread.h>
#include <synch.h>
#include <test.h>
#include <spinlock.h>

#define DEFAULT_THREADS	10
#define DEFAULT_INC	10000

static struct semaphore *tsem = NULL;
static struct lock *thread_lock = NULL;
static struct spinlock thread_spinlock;
static unsigned long counter = 0;

static
void
init_sem(void)
{
	if (tsem==NULL) {
		tsem = sem_create("tsem", 0);
		if (tsem == NULL) {
			panic("threadtest: sem_create failed\n");
		}
	}
}

static
void
destroy_sem(void)
{
	if (tsem != NULL) {
		sem_destroy(tsem);
		tsem = NULL;
	}
}

static
void
init_lock(void)
{
	if (thread_lock == NULL) {
		thread_lock = lock_create("thread_lock");
		if (thread_lock == NULL) {
			panic("threadtest: lock_create failed\n");
		}
	}
}

static
void
destroy_lock(void)
{
	if (thread_lock != NULL) {
		lock_destroy(thread_lock);
		thread_lock = NULL;
	}
}

static
void
unsafecounterthread(void *junk, unsigned long num)
{
	(void)junk;

	for (unsigned long i = 0; i < num; i++) {
		counter++;
	}
	V(tsem);
}

static
void
rununsafethreadcounter(int num_threads, int num_inc)
{
	char name[16];
	int i, result;

	counter = 0;

	for (i=0; i<num_threads; i++) {
		snprintf(name, sizeof(name), "threadtest%d", i);
		result = thread_fork(name, NULL, unsafecounterthread, NULL, num_inc);
		if (result) {
			panic("threadtest: thread_fork failed %s)\n", 
			      strerror(result));
		}
	}

	for (i=0; i<num_threads; i++) {
		P(tsem);
	}
}


int
unsafethreadcounter(int nargs, char **args)
{
	unsigned long num_threads = DEFAULT_THREADS;
	unsigned long num_inc = DEFAULT_INC;
	
	if (nargs > 1)
		num_threads = atoi(args[1]);
	if (nargs > 2)
		num_inc = atoi(args[2]);


	init_sem();
	kprintf("Starting an unsafe thread counter (close your eyes kids!) with %lu threads...\n", num_threads);
	rununsafethreadcounter(num_threads, num_inc);
	kprintf("\nThread test done.\n");
	kprintf("Counter: %lu (should be %lu)\n", counter, num_threads * num_inc);
	destroy_sem();

	return 0;
}

static
void
lockcounterthread(void *junk, unsigned long num)
{
	(void)junk;

	for (unsigned long i = 0; i < num; i++) {
		lock_acquire(thread_lock);
		counter++;
		lock_release(thread_lock);
	}
	V(tsem);
}

static
void
runlockthreadcounter(int num_threads, int num_inc)
{
	char name[16];
	int i, result;

	counter = 0;

	for (i=0; i<num_threads; i++) {
		snprintf(name, sizeof(name), "threadtest%d", i);
		result = thread_fork(name, NULL, lockcounterthread, NULL, num_inc);
		if (result) {
			panic("threadtest: thread_fork failed %s)\n", 
			      strerror(result));
		}
	}

	for (i=0; i<num_threads; i++) {
		P(tsem);
	}
}


int
lockthreadcounter(int nargs, char **args)
{
	unsigned long num_threads = DEFAULT_THREADS;
	unsigned long num_inc = DEFAULT_INC;
	
	if (nargs > 1)
		num_threads = atoi(args[1]);
	if (nargs > 2)
		num_inc = atoi(args[2]);


	init_sem();
	init_lock();
	kprintf("Starting a thread counter using locks with %lu threads...\n", num_threads);
	runlockthreadcounter(num_threads, num_inc);
	kprintf("\nThread test done.\n");
	kprintf("Counter: %lu (should be %lu)\n", counter, num_threads * num_inc);
	destroy_lock();
	destroy_sem();

	return 0;
}

static
void
spinlockcounterthread(void *junk, unsigned long num)
{
	(void)junk;

	for (unsigned long i = 0; i < num; i++) {
		spinlock_acquire(&thread_spinlock);
		counter++;
		spinlock_release(&thread_spinlock);
	}
	V(tsem);
}

static
void
runspinlockthreadcounter(int num_threads, int num_inc)
{
	char name[16];
	int i, result;

	counter = 0;

	for (i=0; i<num_threads; i++) {
		snprintf(name, sizeof(name), "threadtest%d", i);
		result = thread_fork(name, NULL, spinlockcounterthread, NULL, num_inc);
		if (result) {
			panic("threadtest: thread_fork failed %s)\n", 
			      strerror(result));
		}
	}

	for (i=0; i<num_threads; i++) {
		P(tsem);
	}
}


int
spinlockthreadcounter(int nargs, char **args)
{
	unsigned long num_threads = DEFAULT_THREADS;
	unsigned long num_inc = DEFAULT_INC;
	
	if (nargs > 1)
		num_threads = atoi(args[1]);
	if (nargs > 2)
		num_inc = atoi(args[2]);

	init_sem();
	spinlock_init(&thread_spinlock);
	kprintf("Starting a thread counter using spinlocks with %lu threads...\n", num_threads);
	runspinlockthreadcounter(num_threads, num_inc);
	kprintf("\nThread test done.\n");
	kprintf("Counter: %lu (should be %lu)\n", counter, num_threads * num_inc);
	spinlock_cleanup(&thread_spinlock);
	destroy_sem();

	return 0;
}
