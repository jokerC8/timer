all:timer_test

CC := clang
CFLAGS += -g -O0 -Wall -Werror -fPIC
LDFLAGS +=

src := heap_timer.c \
	   heap_timer_test.c

obj := $(patsubst %.c, %.o, $(src))

timer_test:$(obj)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o:%.c
	$(CC) -o $@ -c $< $(CFLAGS)

.PHONY:clean

clean:
	@rm -rf *.o timer_test
