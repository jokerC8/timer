#ifndef __DOIP_TIMER_H__
#define __DOIP_TIMER_H__

struct doip_loop;
typedef struct doip_loop doip_loop_t;

struct doip_timer;
typedef struct doip_timer doip_timer_t;

#define DOIP_TIMER_MIN_CAPACITY   (256)

struct doip_loop {
	unsigned long count;
	unsigned long capacity;
	doip_timer_t **timers;
};

struct doip_timer {
	int valid;
	int once;
	int index;
	double timeout;
	double delay;
	double repeat;
	void *userdata;
	void (*cb)(doip_loop_t *loop, doip_timer_t *timer);
};


doip_loop_t *doip_loop_alloc(int size);

doip_timer_t *doip_timer_alloc(void (*cb)(doip_loop_t *loop, doip_timer_t *timer), double timeout, double delay, unsigned char once);

void doip_timer_start(doip_loop_t *loop, doip_timer_t *timer);

void doip_timer_stop(doip_loop_t *loop, doip_timer_t *timer);

void doip_timer_set_userdata(doip_timer_t *timer, void *userdata);

void doip_timer_destroy(doip_timer_t *timer);

void doip_timer_loop(doip_loop_t *loop);

void doip_timer_loop_forever(doip_loop_t *loop);

void doip_timer_dump(doip_loop_t *loop);

#endif
