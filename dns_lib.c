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


int make_answer(char *query, uint16_t RDLENGTH, uint32_t RDATA) {
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

  //uint16_t RDLENGTH = 0x00;
  memcpy(query + offset, &RDLENGTH, 2);
  offset += 2;

  if (RDLENGTH != 0) {
    memcpy(query + offset, &RDATA, 4);
    offset += 4;
  }

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
    //dbprintf("make_question: p2 != NULL, len = %d\n", len);
    
    memcpy(query + offset, &len, 1);
    ++offset;
    //dbprintf("make_question: push %d\n", len);

    for (i = 0; i < len; i++) {
      memcpy(query + offset, p1 + i, sizeof(char));
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
  memcpy(query + offset, &len, 1);
  ++offset;
  //dbprintf("make_question: push %d\n", len);

  //dbprintf("make_question: last seg, len = %d\n", len);
  for (i = 0; i < len; i++) {
    memcpy(query + offset, p1 + i, sizeof(char));
    //dbprintf("make_question: push %c\n", *(query + offset));
    ++offset;
  }

  //dbprintf("make_question: push str %s\n", query);

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
  //dbprintf("make_question: QTYPE:%d QCLASS:%d\n", *(query+offset-4), *(query+offset-2));

  return offset;

}

int make_head(char *query, uint16_t msg_id, uint16_t flags, uint16_t QDCOUNT, uint16_t ANCOUNT, uint16_t NSCOUNT, uint16_t ARCOUNT) {
  assert(query != NULL);
  //assert(*query != NULL);

  int size = 2; // 2 bytes
  
  memcpy(query, &msg_id, size);
  memcpy(query + size, &flags, size);
  memcpy(query + 2*size, &QDCOUNT, size);
  memcpy(query + 3*size, &ANCOUNT, size);
  memcpy(query + 4*size, &NSCOUNT, size);
  memcpy(query + 5*size, &ARCOUNT, size);
  
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
struct query_t *parse_query(char *query) {
  assert(query != NULL);

  struct query_t *q = NULL;
  q = (struct query_t *)calloc(1, sizeof(struct query_t));

  // header section
  memcpy(&(q->msg_id), query, 2);
  
  memcpy(&(q->flags), query+2, 2);
  
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

  memcpy(&(q->QDCOUNT), query+4, 2);
  
  memcpy(&(q->ANCOUNT), query+6, 2);
  
  // query section
  q->QNAME = (char *)calloc(strlen(query+12)+1, sizeof(char));
  memcpy(q->QNAME, query+12, strlen(query+12));

  char *p = strchr(query+12, '\0');
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


// actually, can also be used to print reply
int print_query(struct query_t *q) {
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

char *make_dns_reply(struct query_t *query, uint32_t ip, int *query_len) {
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
  *query_len = offset;

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
  struct query_t *query = parse_query(query_str);
  print_query(query);


  int reply_len;
  char *reply = make_dns_reply(query, 0x01, &reply_len);
  struct query_t *reply_struct = parse_query(reply);
  print_query(reply_struct);

}

#endif
