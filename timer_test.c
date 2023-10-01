#include "timer.h"
#include <stdio.h>
#include <stdlib.h>

static void timer_callback(doip_loop_t *loop, doip_timer_t *timer)
{
	fprintf(stdout, "[timer:%p] timeout:%f, count:%ld, cap:%ld\n", timer, timer->repeat, loop->count, loop->capacity);
	//doip_timer_stop(loop, timer);
	//doip_timer_destroy(timer);
}

int main(void)
{
	doip_loop_t *loop = NULL;

	loop = doip_loop_alloc(10);
	if (!loop) {
		return 0;
	}

	for (int i = 0; i < 500; i++) {
		doip_timer_t *timer = doip_timer_alloc(timer_callback, 1000 + 10 * i, 0, 0);
		doip_timer_set_userdata(timer, timer);
		doip_timer_start(loop, timer);
	}

	doip_timer_loop_forever(loop);
}
