#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "helper.h"
#include "nameserver.h"

int main(int argc, char *argv[]) {
  if (argc < 6) {
    printf("Usage: ./nameserver [-r] <log> <ip> <port> <servers> <LSAs>\n");
    exit(-1);
  }

  int round_robin;
  char *log;
  char *ip;
  short port;
  char *servers;
  char *LSAs;

  int sock;
  char *query_buf, *reply_buf;
  struct sockaddr_in client_addr, addr;
  socklen_t client_len;
  int recv_ret, send_ret;
  
  if (strcmp(argv[1], "-r") == 0) {
    round_robin = 1;
    log = argv[2];
    ip = argv[3];
    port = atoi(argv[4]);
    servers = argv[5];
    LSAs = argv[6];
  } else {
    round_robin = 0;
    log = argv[1];
    ip = argv[2];
    port = atoi(argv[3]);
    servers = argv[4];
    LSAs = argv[5];
  }


  // sock
  if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    perror("Error! nameserver, socket");
    exit(-1);
  }
  
  addr.sin_family = AF_INET;
  if (inet_aton(ip, &addr.sin_addr) == 0) {
    perror("Error! nameserver, inet_aton\n");
    exit(-1);
  }
  //addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(port);

  if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    perror("nameserver: bind\n");
    exit(-1);
  }
  

  // recvfrom
  query_buf = (char *)calloc(BUF_SIZE, sizeof(char));
  while (1) {
    memset(&client_addr, 0, sizeof(client_addr));
    memset(query_buf, 0, BUF_SIZE);

    printf("nameserver: ready to recvfrom\n");
    recv_ret = recvfrom(sock, query_buf, BUF_SIZE, 0, (struct sockaddr *)&client_addr, &client_len);
    //printf("nameserver: recvd %s\n", query_buf);
    print_query(query_buf);
    
    if (recv_ret == -1) {
      perror("Error! nameserve, recvfrom\n");
      exit(-1);
    }
    
    // udp, a packet is recvd as a whole
    if (recv_ret > 0) {
      dbprintf("nameserver: recved %d bytes, do choose_cnd\n", recv_ret);
      
      reply_buf = choose_cnd(query_buf);
      
      if ((send_ret = sendto(sock, reply_buf, strlen(reply_buf), 0, (struct sockaddr *)&client_addr, client_len)) != strlen(reply_buf)) {
	perror("Error! nameserver, sendto\n");
	exit(-1);
      }
      
      free(reply_buf);
    }

  }
  
  free(query_buf);
}


char *choose_cnd(char *query) {
  assert(query != NULL);
  dbprintf("choose_cnd:\n");
  
  uint16_t msg_id;
  memcpy(&msg_id, query, 2);
  printf("msg_id:%x\n", msg_id);
  
  uint16_t flags;
  memcpy(&flags, query+2, 2);
  
  uint16_t QR = 0x01 << 15;
  QR = QR & flags;
  QR = QR >> 15;
  printf("QR:%d\n", QR);

  uint16_t OPCODE = 0x0f << 11;
  OPCODE = OPCODE & flags;
  OPCODE = OPCODE >> 14;
  printf("OPCODE:%x\n", OPCODE);
  
  uint16_t AA = 0x01 << 10;
  AA = AA & flags;
  AA = AA >> 10;
  printf("AA:%d\n", AA);

  uint16_t RD = 0x01 << 8;
  RD = RD & flags;
  RD = RD >> 8;
  printf("RD:%d\n", RD);

  uint16_t RCODE = 0x0f;
  RCODE = RCODE & flags;
  printf("RCODE:%x\n", RCODE);

  uint16_t QDCOUNT;
  memcpy(&QDCOUNT, query+4, 2);
  printf("QDCOUNT:%d\n", QDCOUNT);
  
  uint16_t ANCOUNT;
  memcpy(&ANCOUNT, query+6, 2);
  printf("ANCOUNT:%d\n", ANCOUNT);
  
  // query section
  char *p = strchr(query+12, '\0');
  //printf("p - query = %ld\n", p - query);
  p += 1;

  char *QNAME = query+12;
  printf("query section: qname:%s\n", QNAME);
  
  short qtype, qclass;
  memcpy(&qtype, p, 2);
  memcpy(&qclass, p+2, 2);
  printf("query section:%s, %x, %x\n", query + 12, qtype, qclass);

  // answer section
  p += 4;
  uint16_t NAME;
  memcpy(&NAME, p, 2);
  printf("answer section: NAME:%x,", NAME);
  p += 2;
  
  uint16_t TYPE;
  memcpy(&TYPE, p, 2);
  printf("TYPE:%x,", TYPE);
  p += 2;
  
  uint16_t CLASS;
  memcpy(&CLASS, p, 2);
  printf("CLASS:%x,", CLASS);
  p += 2;

  uint32_t TTL;
  memcpy(&TTL, p, 4);
  printf("TTL:%x,", TTL);
  p += 4;
  
  uint16_t RDLENGTH;
  memcpy(&RDLENGTH, p, 2);
  printf("RDLENGTH:%x\n", RDLENGTH);
  p += 2;

  // 
  printf("Now, just return 15441\n");
  char * ret = (char *)calloc(1024, sizeof(char));
  
  memcpy(ret, "15441", strlen("15441"));

  return ret;

}


