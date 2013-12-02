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
  int query_len;

  // make query packet
  //dns_query = make_dns_query(node, service);
  dns_query = make_dns_query(node, &query_len);
  print_query(dns_query);
  
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
  if (sendto(sock, dns_query, query_len, 0, (struct sockaddr *)&addr, sizeof(addr)) != query_len) {
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

char *make_dns_query(const char *node, int *query_len) {
  assert(node != NULL);

  //printf("make_dns_query: not imp yet, return \"dns_query\"\n");
  static uint16_t msg_id = 0;
  char *query = (char *)calloc(BUF_SIZE, sizeof(char));
  int offset;

  // make head section
  msg_id += 1;

  uint16_t QR = 0x01 << (15-0);
  uint16_t OPCODE = 0x00 << (15-4);
  uint16_t AA = 0x00 << (15-5);
  uint16_t TC = 0x00;
  uint16_t RD = 0x00;
  uint16_t RA = 0x00;
  uint16_t RCODE = 0x00;
  uint16_t flags = QR | OPCODE | AA | TC | RD | RA | RCODE;

  uint16_t QDCOUNT = get_qdcount(node);

  uint16_t ANCOUNT = 0x00;
  
  offset = make_head(query, msg_id, flags, QDCOUNT, ANCOUNT);

  // make question section
  offset += make_question(query + offset, node);

  // make answer section
  offset += make_answer(query + offset);
  
  *query_len = offset;
  
  return query;

}

int make_answer(char *query) {
  assert(query != NULL);
  
  int offset = 0;

  uint16_t NAME = 0xC00C;
  memcpy(query + offset, &NAME, 2);
  offset += 2;
  
  uint16_t TYPE = 0x01;
  memcpy(query + offset, &TYPE, 2);
  offset += 2;
  
  uint16_t CLASS = 0x01;
  memcpy(query + offset, &CLASS, 2);
  offset += 2;

  uint32_t TTL = 0x00;
  memcpy(query + offset, &TTL, 4);
  offset += 4;

  uint16_t RDLENGTH = 0x00;
  memcpy(query + offset, &RDLENGTH, 2);
  offset += 2;

  //uint16_t RDATA; // not exist
  return offset;
}

int make_question(char *query, const char *node) {
  assert(query != NULL);
  assert(node != NULL);
  
  const char *p1, *p2;
  uint8_t len;
  int i, offset;

  offset = 0;
  p1 = node;
  
  while ((p2 = strchr(p1, '.')) != NULL) {
    len = p2 - p1;
    assert(len <= 0x3f); // ensure label format
    dbprintf("make_question: p2 != NULL, len = %d\n", len);
    
    memcpy(query + offset, &len, 1);
    ++offset;
    printf("make_question: push %d\n", len);

    for (i = 0; i < len; i++) {
      memcpy(query + offset, p1 + i, sizeof(char));
      //dbprintf("make_question: push %c\n", *(query+offset));
      ++offset;
    }

    printf("make_question: push str %s\n", query);

    //
    p1 = p2 + 1;
  }

  // last seg
  len = strlen(p1);
  assert(len <= 0x3f); // ensure label format
  memcpy(query + offset, &len, 1);
  ++offset;
  dbprintf("make_question: push %d\n", len);

  dbprintf("make_question: last seg, len = %d\n", len);
  for (i = 0; i < len; i++) {
    memcpy(query + offset, p1 + i, sizeof(char));
    //dbprintf("make_question: push %c\n", *(query + offset));
    ++offset;
  }

  dbprintf("make_question: push str %s\n", query);

  // end wiht 0x00
  *(query + offset) = 0x00;
  ++offset;

  // qtype, qclass
  uint16_t QTYPE = 0x01;
  uint16_t QCLASS = 0x01;
  memcpy(query + offset, &QTYPE, 2);
  offset += 2;
  memcpy(query + offset, &QCLASS, 2);
  offset += 2;
  dbprintf("make_question: QTYPE:%d QCLASS:%d\n", *(query+offset-4), *(query+offset-2));

  return offset;

}

int make_head(char *query, uint16_t msg_id, uint16_t flags, uint16_t QDCOUNT, uint16_t ANCOUNT) {
  assert(query != NULL);
  //assert(*query != NULL);

  int size = 2; // 2 bytes
  
  memcpy(query, &msg_id, size);
  memcpy(query + size, &flags, size);
  memcpy(query + 2*size, &QDCOUNT, size);
  memcpy(query + 3*size, &ANCOUNT, size);
  
  return size*6;
}

uint16_t get_qdcount(const char *node) {
  assert(node != NULL);

  uint16_t count = 0;

  while ((node = strchr(node, '.')) != NULL) {
    ++count;
    node += 1;
  }

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


#ifdef TEST

int main() {

  int count = get_qdcount("www.google.com");
  assert(count == 3);
  printf("get_qdcount passed test!\n");

  int query_len;
  char *dns_query = make_dns_query("www.google.com", &query_len);
  print_query(dns_query);

  return 0;
}

#endif
