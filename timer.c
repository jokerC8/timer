#include "timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <poll.h>

static double get_system_runtime()
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);

	return ts.tv_sec * 1000 + ts.tv_nsec / 1e6;
}

static int heap_timer_empty(timer_loop_t *loop)
{
	return !(loop->count > 0);
}

static void up(timer_loop_t *loop, unsigned long index)
{
	unsigned long child;
	heap_timer_t *temp;

	while (index / 2) {
		child = index / 2;
		if (loop->timers[child]->timeout > loop->timers[index]->timeout) {
			/* swap */
			loop->timers[child]->index = index;
			loop->timers[index]->index = child;
			temp = loop->timers[index];
			loop->timers[index] = loop->timers[child];
			loop->timers[child] = temp;
		}

		index = child;
	}
}

static void down(timer_loop_t *loop, unsigned long index)
{
	unsigned long father;
	heap_timer_t *temp;

	if (!(loop && index > 0)) {
		return;
	}

	while (index * 2 <= loop->count) {
		father = 2 * index;
		if (loop->count > father && loop->timers[father + 1]->timeout < loop->timers[father]->timeout) {
			father += 1;
		}
		if (loop->timers[father]->timeout < loop->timers[index]->timeout) {
			/* swap */
			loop->timers[father]->index = index;
			loop->timers[index]->index = father;
			temp = loop->timers[index];
			loop->timers[index] = loop->timers[father];
			loop->timers[father] = temp;
		}

		index = father;
	}
}

timer_loop_t *timer_loop_alloc(int size)
{
	timer_loop_t *loop = NULL;

	if (size < DOIP_TIMER_MIN_CAPACITY) {
		size = DOIP_TIMER_MIN_CAPACITY;
	}

	loop = calloc(1, sizeof(timer_loop_t));
	if (loop) {
		loop->count = 0;
		loop->capacity = size;
		loop->timers = calloc(size, sizeof(void *));
		if (!loop->timers) {
			goto nomem;
		}
	}

	return loop;

nomem:
	if (loop) {
		free(loop);
		loop = NULL;
	}
	return NULL;
}

heap_timer_t *heap_timer_alloc(void (*cb)(timer_loop_t *loop, heap_timer_t *timer), double timeout, double delay, unsigned char once)
{
	heap_timer_t *timer = NULL;

	timer = calloc(1, sizeof(*timer));
	if (timer) {
		timer->cb = cb;
		timer->valid = 0;
		timer->sn = 0;
		timer->timeout = 0;
		timer->delay = delay;
		timer->repeat = timeout;
		timer->once = once;
		timer->index = 0;
		timer->userdata = NULL;
	}

	return timer;
}

void heap_timer_start(timer_loop_t *loop, heap_timer_t *timer)
{
	unsigned long index;

	if (!(loop && timer)) {
		return;
	}

	/* if user modify timer's timeout in timer's callback
	 * function, heap_timer_start must be called again, it
	 * will keep the array as a minimal heap.
	 */
	if (timer->valid) {
		index = timer->index;
		up(loop, index);
		down(loop, index);
		return;
	}

	if (loop->count == loop->capacity - 1) {
		heap_timer_t **timers = calloc(1, (loop->capacity + DOIP_TIMER_MIN_CAPACITY)* sizeof(void *));
		memcpy(timers, loop->timers, loop->capacity * sizeof(void *));
		free(loop->timers);
		loop->timers = timers;
		loop->capacity += DOIP_TIMER_MIN_CAPACITY;
	}

	loop->count++;
	loop->timers[loop->count] = timer;
	timer->index = loop->count;
	if (timer->delay > 0) {
		timer->timeout += timer->delay;
	}
	timer->timeout += timer->repeat;
	timer->timeout = get_system_runtime() + timer->timeout;
	timer->valid = 1;
	timer->sn = loop->count;
	up(loop, loop->count);
}

void heap_timer_stop(timer_loop_t *loop, heap_timer_t *timer)
{
	if (!(loop && timer)) {
		return;
	}

	if (timer->valid) {
		loop->timers[timer->index] = loop->timers[loop->count];
		loop->timers[timer->index]->index = timer->index;
		loop->count--;
		down(loop, timer->index);
		timer->valid = 0;
	}
}

void heap_timer_destroy(heap_timer_t *timer)
{
	if (timer) {
		memset(timer, 0, sizeof(heap_timer_t));
		free(timer);
	}
}

void heap_timer_set_userdata(heap_timer_t *timer, void *userdata)
{
	if (timer) {
		timer->userdata = userdata;
	}
}

void *heap_timer_userdata(heap_timer_t *timer)
{
	return (!!timer) ? timer->userdata : NULL;
}

void heap_timer_loop(timer_loop_t *loop)
{
	heap_timer_t *timer;
	unsigned long current;

	while (1) {
		if (heap_timer_empty(loop)) {
			return;
		}

		timer = loop->timers[1];
		current = get_system_runtime();
		if (timer->timeout > current) {
			break;
		}
		if (timer->once) {
			heap_timer_stop(loop, timer);

		} else {
			timer->timeout = current + timer->repeat;
			down(loop, timer->index);
		}
		if (timer->cb) {
			timer->cb(loop, timer);
		}
	}
}

void heap_timer_loop_forever(timer_loop_t *loop)
{
	while (1) {
		heap_timer_loop(loop);
		poll(0, 0, 10);
	}
}
