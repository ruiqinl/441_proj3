CC = gcc
CFLAGS = -DDEBUG -Wall -g

OBJS = proxy.o http_parser.o http_replyer.o helper.o
BINS = proxy

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

all: $(BINS)

run:
	./proxy logfile 0.5 8889 1.0.0.1 5.0.0.1 9999 3.0.0.1

proxy: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf $(OBJS) $(BINS) *~


