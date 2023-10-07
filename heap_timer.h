#ifndef __HEAP_TIMER_H__
#define __HEAP_TIMER_H__

struct timer_loop;
typedef struct timer_loop timer_loop_t;

struct heap_timer;
typedef struct heap_timer heap_timer_t;

#define DOIP_TIMER_MIN_CAPACITY   (256)

struct timer_loop {
	unsigned long count;
	unsigned long capacity;
	heap_timer_t **timers;
};

struct heap_timer {
	int valid;
	int once;
	unsigned long sn;
	unsigned long index;
	double timeout;
	double delay;
	double repeat;
	void *userdata;
	void (*cb)(timer_loop_t *loop, heap_timer_t *timer);
};


timer_loop_t *timer_loop_alloc(int size);

heap_timer_t *heap_timer_alloc(void (*cb)(timer_loop_t *loop, heap_timer_t *timer), double timeout, double delay, unsigned char once);

void heap_timer_start(timer_loop_t *loop, heap_timer_t *timer);

void heap_timer_stop(timer_loop_t *loop, heap_timer_t *timer);

void heap_timer_set_userdata(heap_timer_t *timer, void *userdata);

void *heap_timer_userdata(heap_timer_t *timer);

void heap_timer_destroy(heap_timer_t *timer);

void heap_timer_loop(timer_loop_t *loop);

void heap_timer_loop_forever(timer_loop_t *loop);

#endif
