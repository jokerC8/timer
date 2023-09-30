#include "timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <poll.h>

static unsigned long get_system_runtime()
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);

	return ts.tv_sec * 1000 + ts.tv_nsec / 1e6;
}

doip_loop_t *doip_loop_alloc(int size)
{
	doip_loop_t *loop;

	if (size < DOIP_TIMER_MIN_CAPACITY) {
		size = DOIP_TIMER_MIN_CAPACITY;
	}

	loop = malloc(sizeof(doip_loop_t));
	if (!loop) {
		goto nomem;
	}

	loop->count = 0;
	loop->capacity = size;
	loop->timers = malloc(size * sizeof(void *));
	if (!loop->timers) {
		goto nomem;
	}

	return loop;

nomem:
	if (loop) {
		free(loop);
		loop = NULL;
	}
	return NULL;
}

doip_timer_t *doip_timer_alloc(void (*cb)(doip_loop_t *loop, doip_timer_t *timer), double timeout, double delay, unsigned char once)
{
	doip_timer_t *timer;

	timer = malloc(sizeof(*timer));
	if (!timer) {
		return NULL;
	}

	timer->cb = cb;
	timer->valid = 0;
	timer->timeout = 0;
	timer->delay = delay;
	timer->repeat = timeout;
	timer->once = once;
	timer->index = 0;
	timer->userdata = NULL;

	return timer;
}

void doip_timer_set_userdata(doip_timer_t *timer, void *userdata)
{
	if (!timer) {
		return;
	}

	timer->userdata = userdata;
}

static void up(doip_loop_t *loop, unsigned long index)
{
	unsigned long child;
	doip_timer_t *temp;

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
		index /= 2;
	}
}

static void down(doip_loop_t *loop, unsigned long index)
{
	unsigned long father;
	doip_timer_t *temp;

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

void doip_timer_start(doip_loop_t *loop, doip_timer_t *timer)
{
	if (!(loop && timer)) {
		return;
	}

	if (loop->count == loop->capacity - 1) {
		doip_timer_t **timers = calloc(1, (loop->capacity + DOIP_TIMER_MIN_CAPACITY)* sizeof(void *));
		memcpy(timers, loop->timers, loop->capacity * sizeof(void *));
		free(loop->timers);
		loop->timers = timers;
		loop->capacity = loop->capacity + DOIP_TIMER_MIN_CAPACITY;
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
	up(loop, loop->count);
}

void doip_timer_stop(doip_loop_t *loop, doip_timer_t *timer)
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

void doip_timer_destroy(doip_timer_t *timer)
{
	if (timer) {
		memset(timer, 0, sizeof(doip_timer_t));
		free(timer);
	}
}

static int doip_timer_empty(doip_loop_t *loop)
{
	return !(loop->count > 0);
}

void doip_timer_loop(doip_loop_t *loop)
{
	doip_timer_t *timer;

	while (1) {
		if (doip_timer_empty(loop)) {
			return;
		}

		timer = loop->timers[1];
		if (timer->timeout > get_system_runtime()) {
			break;
		}
		if (timer->once) {
			doip_timer_stop(loop, timer);
		}
		if (timer->cb) {
			timer->cb(loop, timer->userdata);
		}
		if (!timer->once) {
			timer->timeout += timer->repeat;
			down(loop, timer->index);
		}
	}
}

void doip_timer_loop_forever(doip_loop_t *loop)
{
	while (1) {
		doip_timer_loop(loop);
		poll(0, 0, 10);
	}
}
