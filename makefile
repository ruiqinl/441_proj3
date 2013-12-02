CC = gcc
CFLAGS = -DDEBUG -Wall -g

OBJS_proxy = proxy.o http_parser.o http_replyer.o helper.o mydns.o 
OBJS_name = nameserver.o
BINS_TEST = mydns_test
BINS = proxy nameserver 

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

all: $(BINS) $(BINS_TEST)

run1cp1:
	./proxy logfile1 0.5 8888 1.0.0.1 5.0.0.1 9999 3.0.0.1
run2cp1:
	./proxy logfile2 0.5 8889 2.0.0.1 5.0.0.1 9999 4.0.0.1
run1cp2:
	./proxy logfile1 0.5 8888 1.0.0.1 5.0.0.1 9999
run2cp2:
	./proxy logfile2 0.5 8889 2.0.0.1 5.0.0.1 9999

runbothcp1:
	./proxy logfile1 0.5 8888 1.0.0.1 5.0.0.1 9999 3.0.0.1 >printf1.txt & ./proxy logfile2 0.5 8889 2.0.0.1 5.0.0.1 9999 4.0.0.1 > printf2.txt &

runbothcp2:
	./proxy logfile1 0.5 8888 1.0.0.1 5.0.0.1 9999 >printf1.txt & ./proxy logfile2 0.5 8889 2.0.0.1 5.0.0.1 9999 > printf2.txt &


rundns:
	./nameserver -r nameserver_log 5.0.0.1 9999 ./topos/topo1/topo1.servers ./topos/topo1/topo1.lsa

proxy: $(OBJS_proxy)
	$(CC) $(CFLAGS) $^ -o $@

nameserver: $(OBJS_name)
	$(CC) $(CFLAGS) $^ -o $@

mydns_test:
	$(CC) $(CFLAGS) -D_TEST_MYDNS_ mydns.c helper.c -o $@

clean:
	rm -rf $(OBJS_proxy) $(OBJS_name) $(BINS) *~


