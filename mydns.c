#include <stdio.h>
#include <assert.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include "helper.h"
#include "mydns.h"

static const char *dns_ip = NULL;
static unsigned int dns_port = 0;

int init_mydns(const char *ip, unsigned int port) {
  assert(ip != NULL);
  assert(port > 0);

  dns_ip = ip;
  dns_port = port;

  return 0;
}

int resolve(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res) {
  assert(node != NULL);
  assert(service != NULL);
  assert(res != NULL);

  struct sockaddr_in addr;
  char *dns_query = NULL;
  char *dns_reply = NULL;
  int sock;
  int ret;  

  // make query packet
  //dns_query = make_dns_query(node, service);
  dns_query = make_dns_query(node);
  
  // udp socket to dns server
  printf("resolve: dns_ip:%s, dns_port:%d\n", dns_ip, dns_port);
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  if (inet_aton(dns_ip, &addr.sin_addr) == 0) {
    perror("Error! main, inet_aton");
    exit(-1);
  }
  addr.sin_port = htons(dns_port);
  
  if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    perror("Error! mydns, socket");
    exit(-1);
  }

  // send
  printf("mydns: ready to sendto\n");
  if (sendto(sock, dns_query, strlen(dns_query), 0, (struct sockaddr *)&addr, sizeof(addr)) != strlen(dns_query)) {
    perror("Error! mydns, sendto, maybe use while");
    exit(-1);
  }
  printf("mydns: sent %s\n", dns_query);

  // recv
  dns_reply = (char *)calloc(BUF_SIZE, sizeof(char));
  //dns_reply_p = dns_reply;
  //buf_size = BUF_SIZE;

  ret = recvfrom(sock, dns_reply, BUF_SIZE, 0, NULL, NULL);
  printf("mydns: recvd %s\n", dns_reply);

  if (ret == -1) {
    perror("Error! mydns, recvfrom");
    exit(-1);
  }
  
  // parse it 
  *res = (struct addrinfo *)calloc(1, sizeof(struct addrinfo));
  (*res)->ai_family = AF_INET;
  (*res)->ai_addr = parse_dns_reply(dns_reply);
  (*res)->ai_next = NULL;

  free(dns_reply);
  return 0;
}

char *make_dns_query(const char *node) {
  assert(node != NULL);

  //printf("make_dns_query: not imp yet, return \"dns_query\"\n");
  static uint16_t msg_id = 0;
  char *query = (char *)calloc(BUF_SIZE, sizeof(char));

  msg_id += 1;

  uint16_t QR = 0x01 << (15-0);
  uint16_t OPCODE = 0x00 << (15-4);
  uint16_t AA = 0x00 << (15-5);
  uint16_t TC = 0x00;
  uint16_t RD = 0x00;
  uint16_t RA = 0x00;
  uint16_t RCODE = 0x00;
  uint16_t flags = QR & OPCODE & AA & TC & RD &RA & RCODE;

  uint16_t QDCOUNT = get_qdcount(node);

  uint16_t ANCOUNT = 0x00;
  
  make_head(&query, msg_id, flags, QDCOUNT, ANCOUNT);
  
  
  return "dns_query";

}

int make_head(char **query, uint16_t msg_id, uint16_t flags, uint16_t QDCOUNT, uint16_t ANCOUNT) {
  assert(query != NULL);
  assert(*query != NULL);

  int size = 2; // 2 bytes

  memcpy(*query, &msg_id, size);
  memcpy(*query + size, &flags, size);
  memcpy(*query + 2*size, &QDCOUNT, size);
  memcpy(*query + 3*size, &ANCOUNT, size);
  
  return size*6;
}

uint16_t get_qdcount(const char *node) {
  assert(node != NULL);

  uint16_t count = 0;
  
  while ((node = strchr(node, '.')) != NULL) 
    ++count;

  count += 1;

  return count;
}

struct sockaddr *parse_dns_reply(char *dns_reply) {
  assert(dns_reply != NULL);
  
  struct sockaddr_in *addr;
  
  addr = (struct sockaddr_in *)calloc(1, sizeof(struct sockaddr_in));

  addr->sin_family = AF_INET;
  if (inet_aton("3.0.0.1", &(addr->sin_addr)) == 0) {
    perror("Error! main, inet_aton");
    exit(-1);
  }
  addr->sin_port = htons(8080);
  
  printf("parse_dns_reply: not imp yet, return sockaddr with sin_addr 3.0.0.1\n");
  
  return (struct sockaddr *)addr;
}


#ifdef _TEST_MYDNS_

int main() {
  
  int count = get_qdcount("www.google.com");
  assert(count == 3);

}

#endif
