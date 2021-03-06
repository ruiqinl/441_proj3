#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <assert.h>

#include "helper.h"
#include "http_replyer.h"
#include "http_parser.h"


int main(int argc, char *argv[]){

    int listen_sock;
    
    struct sockaddr_in listen_addr;
    int backlog = 5;
    int sock;
    //char buf[BUF_SIZE];
    int numbytes;
    struct sockaddr_in cli_addr;
    socklen_t cli_size;

    int listen_port;// = 8888;
    char *fake_ip = NULL;
    char *dns_ip = NULL;
    int dns_port;
    char *www_ip = NULL;

    struct sockaddr_in server_addr;
    int sock2server;

    int maxfd;
    fd_set read_fds, write_fds;
    fd_set master_read_fds, master_write_fds;

    struct buf* buf_pts[MAX_SOCK];
    int i, recv_ret;
    
    
    // parse argv
    if (argc < 8) {
	printf("Usage: ./proxy <log> <alpha> <listen_port> <fake-ip> <dns-ip> <dns-port> <www-ip>\n");
	return -1;
    }

    listen_port = atoi(argv[3]);
    fake_ip = argv[4];
    dns_ip = argv[5];
    dns_port = atoi(argv[6]);
    www_ip = argv[7];

    // browser side of proxy
    listen_sock = socket(AF_INET, SOCK_STREAM, 0);

    memset(&listen_addr, 0, sizeof(listen_addr));
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    listen_addr.sin_port = htons(listen_port);

    if (bind(listen_sock, (struct sockaddr *)&listen_addr, sizeof(listen_addr)) == -1) {
	perror("Error! main, bind");
	exit(-1);
    }

    if (listen(listen_sock, backlog) == -1) {
	perror("Error! main, listen");
	exit(-1);
    }

    maxfd = listen_sock;
    FD_ZERO(&master_read_fds);
    FD_ZERO(&master_write_fds);
    FD_SET(listen_sock, &master_read_fds);
    
    // server side of proxy: addr
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if (inet_aton(www_ip, &server_addr.sin_addr) == 0) {
	perror("Error! main, inet_aton");
	exit(-1);
    }
    server_addr.sin_port = htons(8080);
    
    // server side of proxy: whatever, get f4m first
    if((sock2server = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	perror("Error! main, socket, socket2server");
	exit(-1);
    }
    
    if (connect(sock2server, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
	perror("Error! main, connect, socket2server");
	exit(-1);
    }
    
    getf4m(sock2server);
    
    // receive connection from browser
    
    while (1) {
	
	read_fds = master_read_fds;
	write_fds = master_write_fds;
	
	if (select(maxfd+1, &read_fds, &write_fds, NULL, NULL) == -1) {
	    perror("Error! proxy, select");
	    close_socket(sock);
	    return(-1);
	}

	for (i = 0; i <= maxfd; i++) {
	    
	    // check read_fds
	    if (FD_ISSET(i, &read_fds)) {
		
		if (i == listen_sock) {

		    printf("proxy: received new connection from browser\n");

		    if((sock = accept(listen_sock, (struct sockaddr *)&cli_addr, &cli_size)) == -1){
			perror("Error! proxy, accpet");
			exit(-1);
		    }
		    
		    FD_SET(sock, &master_read_fds);
		    buf_pts[sock] = (struct buf*)calloc(1, sizeof(struct buf));
		    init_buf(buf_pts[sock], sock, "/var/www", &cli_addr, i);
		    printf("buf_pts[%d] allocated, rbuf_free_size:%d\n", sock, buf_pts[sock]->rbuf_free_size);

		    // track maxfd 
		    if (sock > maxfd)
			maxfd = sock;
		    
		} else {
		    
		    printf("proxy: received bytes from browser/server\n");
		    
		    recv_ret = recv_request(i, buf_pts[i]); //recv_ret -1: recv error; 0: recv 0; 1: recv some bytes 
		    printf("===========================================================\n");
		    printf("proxy: recv_request from sock %d, recv_ret is %d\n", i, recv_ret);

		    if (recv_ret == 1){
			parse_request(buf_pts[i]);
			dbprint_queue(buf_pts[i]->req_queue_p);

			if (buf_pts[i]->req_queue_p->req_count > 0) {
			    printf("proxy: what to do now??\n");
			}
		    }
			    

		    FD_CLR(i, &master_read_fds);
		}
	    }

	    // check write_fds
	    if (FD_ISSET(i, &write_fds)) {
		
		printf("proxy: write bytes to browser/server\n");
	    }
	}

    }
    
    
    close(sock);
    close(listen_sock);
    
    return 0;

}
