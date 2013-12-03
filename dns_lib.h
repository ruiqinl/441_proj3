#ifndef _DNS_LIB_H
#define _DNS_LIB_H

struct query_t {

  // header section
  uint16_t msg_id;
  uint16_t flags;
  uint16_t QR;
  uint16_t OPCODE;
  uint16_t AA;
  uint16_t RD;
  uint16_t RCODE;
  uint16_t QDCOUNT;
  uint16_t ANCOUNT;
  
  // query section
  char *QNAME;
  uint16_t QTYPE;
  uint16_t QCLASS;

  // answer section
  uint16_t NAME;
  uint16_t TYPE;
  uint16_t CLASS;
  uint32_t TTL;
  uint16_t RDLENGTH;

};


/**
 * Helper function, make header section
 *
 * @param query The pointer to the buffer to save query
 * @param msg_id Message ID
 * @param flags QR & OPCODE & AA & TC &RD &RA &RCODE
 * @param QDCOUNT 
 * @param ANCOUNT
 *
 * @return size of header section, which is always 12 bytes
 */
int make_head(char *query, uint16_t msg_id, uint16_t flags, uint16_t QDCOUNT, uint16_t ANCOUNT, uint16_t NSCOUNT, uint16_t ARCOUNT);

/**
 * Helper function, return the number of entries in the question section
 *
 * @param node The query string
 *
 * @return number of entries of string
 */
uint16_t get_qdcount(const char *node);


/**
 * Helper function, make question section of the query
 *
 * @param query The allocated buffer to store question section
 * @param node The node to request
 *
 * @return the length of the question
 */
int make_question(char *query, const char *node);

/**
 * Helper function, make answer section of the query
 *
 * @param query The allocated buffer to store answer section
 *
 * @return the length of the answer
 */
int make_answer(char *query, uint16_t RLENGTH, uint32_t RDATA);


/**
 * Print the query
 *
 * @param query The struct queryt_t which contains all fields of query
 *
 * @return 0
 */
int print_query(struct query_t *query);

/**
 * Parse query string, fill all fields into struct query_t
 *
 * @param query The query string
 *
 * @return the pointer to the filled struct query
 */
struct query_t *parse_query(char *query);

/**
 * Make dns reply packet
 *
 * @param ip The ip which should be contained in the reply
 * @param query The received query, whose question part should be contained in reply
 *
 * @return char * to the reply packet
 */
char *make_dns_reply(struct query_t *query, uint32_t ip, int *query_len);

/**
 * Helper function, make dns query packet based on the node param. It's allocated inside, and should be freed by caller
 *
 * @param node The hostname to resolve
 * @param query_len Return the length of the query. '\0' is everywhere inside the query, strlen does not help a lot.
 *
 * @return char * to the generated dns query string
 */
char *make_dns_query(const char *node, int *query_len);

/**
 * Helper function, parse reply from dns server.
 *
 * @param dns_reply The reply from dns server
 *
 * @return sockaddr based the parse of dns_reply
 */
struct sockaddr *parse_dns_reply(char *dns_reply);

/**
 * Recover node from QNAME
 */
char *recover_node(char *QNAME);

#endif
