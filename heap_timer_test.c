#include "heap_timer.h"
#include <stdio.h>
#include <stdlib.h>

static void timer_callback1(timer_loop_t *loop, heap_timer_t *timer)
{
	fprintf(stdout, "[timer:%p] timeout:%f, count:%ld, cap:%ld, sn:%ld\n", timer, timer->repeat, loop->count, loop->capacity, timer->sn);
	//heap_timer_stop(loop, timer);
	//heap_timer_destroy(timer);
}

static void timer_callback(timer_loop_t *loop, heap_timer_t *timer)
{
	fprintf(stdout, "[timer:%p] timeout:%f, count:%ld, cap:%ld, sn:%ld\n", timer, timer->repeat, loop->count, loop->capacity, timer->sn);
	heap_timer_t *timer1 = heap_timer_alloc(timer_callback1, 10, 0, 0);
	heap_timer_start(loop, timer1);
}

int main(void)
{
	timer_loop_t *loop = NULL;

	loop = timer_loop_alloc(10);
	if (!loop) {
		return 0;
	}

	for (int i = 0; i < 500; i++) {
		heap_timer_t *timer = heap_timer_alloc(timer_callback, 100 * (i + 1), 0, 0);
		heap_timer_start(loop, timer);
	}

	heap_timer_loop_forever(loop);
}
