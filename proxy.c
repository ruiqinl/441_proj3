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


#define BUF_SIZE 2048

int main(){

    int listen_sock;
    int listen_port = 8888;
    struct sockaddr_in listen_addr;
    int backlog = 5;
    int sock;
    char buf[BUF_SIZE];
    int numbytes;
    struct sockaddr_in cli_addr;
    socklen_t cli_size;
    
    
    listen_sock = socket(AF_INET, SOCK_STREAM, 0);

    memset(&listen_addr, 0, sizeof(listen_addr));
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    listen_addr.sin_port = htons(listen_port);

    if (bind(listen_sock, (struct sockaddr *)&listen_addr, sizeof(listen_addr)) == -1) {
	perror("Error! main, bind");
	exit(-1);
    }

    listen(listen_sock, backlog);
    
    if((sock = accept(listen_sock, (struct sockaddr *)&cli_addr, &cli_size)) == -1){
	perror("Error! main, accpet");
	exit(-1);
    }

    
    if ((numbytes = recv(sock, buf, BUF_SIZE, 0)) == -1) {
	perror("Error! main, recv\n");
	exit(-1);
    }

    printf("proxy receives from browser:%s\n", buf);
    
    close(sock);
    close(listen_sock);
    
    return 0;

}
