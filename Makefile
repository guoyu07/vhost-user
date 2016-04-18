SRCS := $(wildcard *.c)
HDRS := $(wildcard *.h)
OBJS := $(patsubst %.c, %.o, $(SRCS))

all: app

%.o: %.c $(HDRS)
	gcc -g -c $<

app: $(OBJS) $(HDRS)
	gcc -g -o $@ $^ -lpthread

clean:
	rm -f *.o app

.PHONY: all clean
