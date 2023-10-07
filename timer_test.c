#include "timer.h"
#include <stdio.h>
#include <stdlib.h>

static void timer_callback(timer_loop_t *loop, heap_timer_t *timer)
{
	fprintf(stdout, "[timer:%p] timeout:%f, count:%ld, cap:%ld, sn:%ld\n", timer, timer->repeat, loop->count, loop->capacity, timer->sn);
	//doip_timer_stop(loop, timer);
	//doip_timer_destroy(timer);
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
		heap_timer_set_userdata(timer, timer);
		heap_timer_start(loop, timer);
	}

	heap_timer_loop_forever(loop);
}
