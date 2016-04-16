SRCS := $(wildcard *.c)
HDRS := $(wildcard *.h)
OBJS := $(patsubst %.c, %.o, $(SRCS))

all: app

app: $(OBJS) $(HDRS)
	gcc -g -o $@ $^

clean:
	rm -f *.o app

.PHONY: all clean
