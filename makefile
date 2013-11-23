CC = gcc
CFLAGS = -DDEBUG -Wall -g

OBJS = proxy.o http_parser.o http_replyer.o helper.o
BINS = proxy

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

all: $(BINS) 

run1:
	./proxy logfile1 0.5 8888 1.0.0.1 5.0.0.1 9999 3.0.0.1
run2:
	./proxy logfile2 0.5 9999 2.0.0.1 5.0.0.1 9999 4.0.0.1
runboth:
	./proxy logfile1 0.5 8888 1.0.0.1 5.0.0.1 9999 3.0.0.1 &; ./proxy logfile 0.5 9999 2.0.0.1 5.0.0.1 9999 4.0.0.1 &

proxy: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf $(OBJS) $(BINS) *~


