CC = gcc
CFLAGS = -DDEBUG -Wall -g

OBJS = proxy.o
BINS = proxy

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

all: $(BINS)

proxy: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf $(OBJS) $(BINS) *~


