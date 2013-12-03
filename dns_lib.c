#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "helper.h"
#include "dns_lib.h"


char *make_dns_query(const char *node, int *query_len) {
  assert(node != NULL);

  //printf("make_dns_query: not imp yet, return \"dns_query\"\n");
  static uint16_t msg_id = 0;
  char *query = (char *)calloc(BUF_SIZE, sizeof(char));
  int offset;

  // make head section
  msg_id += 1;

  uint16_t QR = 0x00;
  uint16_t OPCODE = 0x00;
  uint16_t AA = 0x00;
  uint16_t TC = 0x00;
  uint16_t RD = 0x00;
  uint16_t RA = 0x00;
  uint16_t RCODE = 0x00;
  uint16_t flags = QR | OPCODE | AA | TC | RD | RA | RCODE;

  uint16_t QDCOUNT = get_qdcount(node);
  uint16_t ANCOUNT = 0x00;
  uint16_t NSCOUNT = 0x00;
  uint16_t ARCOUNT = 0x00;
  
  offset = make_head(query, msg_id, flags, QDCOUNT, ANCOUNT, NSCOUNT, ARCOUNT);

  // make question section
  offset += make_question(query + offset, node);

  // make answer section
  uint16_t QDLENGTH = 0x00;
  uint32_t QDATA = 0x00; // it's not used
  offset += make_answer(query + offset, QDLENGTH, QDATA);
  
  *query_len = offset;
  
  return query;

}


int make_answer(char *dns, uint16_t RDLENGTH, uint32_t RDATA) {
  assert(dns != NULL);
  
  int offset = 0;

  uint16_t NAME = 0xC00C;
  memcpy(dns + offset, &NAME, 2);
  offset += 2;
  
  uint16_t TYPE = 0x01;
  memcpy(dns + offset, &TYPE, 2);
  offset += 2;
  
  uint16_t CLASS = 0x01;
  memcpy(dns + offset, &CLASS, 2);
  offset += 2;

  uint32_t TTL = 0x00;
  memcpy(dns + offset, &TTL, 4);
  offset += 4;

  //uint16_t RDLENGTH = 0x00;
  memcpy(dns + offset, &RDLENGTH, 2);
  offset += 2;

  if (RDLENGTH != 0) {
    memcpy(dns + offset, &RDATA, 4);
    offset += 4;
  }

  return offset;
}

int make_question(char *dns, const char *node) {
  assert(dns != NULL);
  assert(node != NULL);
  
  const char *p1, *p2;
  uint8_t len;
  int i, offset;

  offset = 0;
  p1 = node;
  
  while ((p2 = strchr(p1, '.')) != NULL) {
    len = p2 - p1;
    assert(len <= 0x3f); // ensure label format
    //dbprintf("make_question: p2 != NULL, len = %d\n", len);
    
    memcpy(dns + offset, &len, 1);
    ++offset;
    //dbprintf("make_question: push %d\n", len);

    for (i = 0; i < len; i++) {
      memcpy(dns + offset, p1 + i, sizeof(char));
      //dbprintf("make_question: push %c\n", *(query+offset));
      ++offset;
    }

    //printf("make_question: push str %s\n", query);

    //
    p1 = p2 + 1;
  }

  // last seg
  len = strlen(p1);
  assert(len <= 0x3f); // ensure label format
  memcpy(dns + offset, &len, 1);
  ++offset;
  //dbprintf("make_question: push %d\n", len);

  //dbprintf("make_question: last seg, len = %d\n", len);
  for (i = 0; i < len; i++) {
    memcpy(dns + offset, p1 + i, sizeof(char));
    //dbprintf("make_question: push %c\n", *(query + offset));
    ++offset;
  }

  //dbprintf("make_question: push str %s\n", query);

  // end wiht 0x00
  *(dns + offset) = 0x00;
  ++offset;

  // qtype, qclass
  uint16_t QTYPE = 0x01;
  uint16_t QCLASS = 0x01;
  memcpy(dns + offset, &QTYPE, 2);
  offset += 2;
  memcpy(dns + offset, &QCLASS, 2);
  offset += 2;
  //dbprintf("make_question: QTYPE:%d QCLASS:%d\n", *(query+offset-4), *(query+offset-2));

  return offset;

}

int make_head(char *dns, uint16_t msg_id, uint16_t flags, uint16_t QDCOUNT, uint16_t ANCOUNT, uint16_t NSCOUNT, uint16_t ARCOUNT) {
  assert(dns != NULL);

  int size = 2; // 2 bytes
  
  memcpy(dns, &msg_id, size);
  memcpy(dns + size, &flags, size);
  memcpy(dns + 2*size, &QDCOUNT, size);
  memcpy(dns + 3*size, &ANCOUNT, size);
  memcpy(dns + 4*size, &NSCOUNT, size);
  memcpy(dns + 5*size, &ARCOUNT, size);
  
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


//
struct dns_t *parse_dns(char *dns) {
  assert(dns != NULL);

  struct dns_t *q = NULL;
  q = (struct dns_t *)calloc(1, sizeof(struct dns_t));

  // header section
  memcpy(&(q->msg_id), dns, 2);
  
  memcpy(&(q->flags), dns+2, 2);
  
  q->QR = 0x01 << 15;
  q->QR = q->QR & q->flags;
  q->QR = q->QR >> 15;

  q->OPCODE = 0x0f << 11;
  q->OPCODE = q->OPCODE & q->flags;
  q->OPCODE = q->OPCODE >> 14;
  
  q->AA = 0x01 << 10;
  q->AA = q->AA & q->flags;
  q->AA = q->AA >> 10;

  q->RD = 0x01 << 8;
  q->RD = q->RD & q->flags;
  q->RD = q->RD >> 8;

  q->RCODE = 0x0f;
  q->RCODE = q->RCODE & q->flags;

  memcpy(&(q->QDCOUNT), dns+4, 2);
  
  memcpy(&(q->ANCOUNT), dns+6, 2);
  
  // query section
  q->QNAME = (char *)calloc(strlen(dns+12)+1, sizeof(char));
  memcpy(q->QNAME, dns+12, strlen(dns+12));

  char *p = strchr(dns+12, '\0');
  p += 1;
  
  memcpy(&(q->QTYPE), p, 2);
  memcpy(&(q->QCLASS), p+2, 2);
  p += 4;

  // answer section
  memcpy(&(q->NAME), p, 2);
  p += 2;
  
  memcpy(&(q->TYPE), p, 2);
  p += 2;
  
  memcpy(&(q->CLASS), p, 2);
  p += 2;

  memcpy(&(q->TTL), p, 4);
  p += 4;
  
  memcpy(&(q->RDLENGTH), p, 2);
  p += 2;

  return q;
  
}


int print_dns(struct dns_t *q) {
  assert(q != NULL);
  printf("print_query/reply:\n");
  
  // header section
  printf("header section: ");
  printf("msg_id:%x, ", q->msg_id);
  printf("QR:%d, ", q->QR);
  printf("OPCODE:%x, ", q->OPCODE);
  printf("AA:%d, ", q->AA);
  printf("RD:%d, ", q->RD);
  printf("RCODE:%x, ", q->RCODE);
  printf("QDCOUNT:%d, ", q->QDCOUNT);
  printf("ANCOUNT:%d\n", q->ANCOUNT);
  // query section
  printf("query section: ");
  printf("QNAME:%s, ", q->QNAME);
  printf("QTYPE:%x, QCLASS:%x\n", q->QTYPE, q->QCLASS);
  // answer section
  printf("answer section: ");
  printf("NAME:%x, ", q->NAME);
  printf("TYPE:%x, ", q->TYPE);
  printf("CLASS:%x, ", q->CLASS);
  printf("TTL:%x,", q->TTL);
  printf("RDLENGTH:%x\n", q->RDLENGTH);
  
  return 0;
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

char *make_dns_reply(struct dns_t *query, uint32_t ip, int *reply_len) {
  assert(query != NULL);
  assert(reply_len != NULL);
  assert(ip != 0x00);
  
  char *reply = (char *)calloc(BUF_SIZE, sizeof(char));
  int offset = 0;
  char *node = NULL;

  // make head section
  uint16_t QR = 0x01 << (15-0);;
  uint16_t OPCODE = 0x00;
  uint16_t AA = 0x01 << (15-5);
  uint16_t TC = 0x00;
  uint16_t RD = 0x00;
  uint16_t RA = 0x00;
  uint16_t RCODE = 0x00;
  uint16_t flags = QR | OPCODE | AA | TC | RD | RA | RCODE;

  uint16_t QDCOUNT = 0x00;
  uint16_t ANCOUNT = 0x01; // ???
  uint16_t NSCOUNT = 0x00;
  uint16_t ARCOUNT = 0x00;

  uint16_t RDLENGTH = 0x04; // 4 bytes
  uint32_t RDATA = ip;
  offset = make_head(reply, query->msg_id, flags, QDCOUNT, ANCOUNT, NSCOUNT, ARCOUNT);
  node = recover_node(query->QNAME);
  offset += make_question(reply + offset, node); // same as original question
  offset += make_answer(reply + offset, RDLENGTH, RDATA);
  
  // return length
  *reply_len = offset;

  return reply;
}


char *recover_node(char *QNAME) {
  
  int len;
  char *node = NULL;
  char size_buf[1];
  int size, count;
  int n_offset, q_offset;

  len = strlen(QNAME);
  node = (char *)calloc(len+1, sizeof(char));

  n_offset = 0;
  q_offset = 0;
  size = 0;

  do {
    memset(size_buf, 0, sizeof(size_buf));
    memcpy(size_buf, QNAME + q_offset, 1);
    ++q_offset;

    size = (int)*size_buf;
    //dbprintf("??size:%d\n", size);
    node[n_offset] = '.';
    ++n_offset;
    for (count = 0; count < size; count++) {
      node[n_offset] = QNAME[q_offset + count];
      //dbprintf("??node[%d]:%c\n", n_offset, node[n_offset]);
      ++n_offset;
    }
    q_offset += size;

  } while (n_offset < len-1);
  
  memmove(node, node+1, len-1);
  node[len-1] = '\0';
  dbprintf("recover_node: QNAME:%s, recovered node:%s\n", QNAME, node);

  return node;
}


#ifdef TEST

int main(){

  int len;
  char *query_str = make_dns_query("www.google.com", &len);
  struct dns_t *query = parse_dns(query_str);
  print_dns(query);


  int reply_len;
  char *reply = make_dns_reply(query, 0x01, &reply_len);
  struct dns_t *reply_struct = parse_dns(reply);
  print_dns(reply_struct);

}

#endif
