#ifndef _MYDNS_H_
#define _MYDNS_H_

#include <netdb.h>

/**
 * Initialize your client DNS library with the IP address and port number of
 * your DNS server.
 *
 * @param  dns_ip  The IP address of the DNS server.
 * @param  dns_port  The port number of the DNS server.
 *
 * @return 0 on success, -1 otherwise
 */
int init_mydns(const char *dns_ip, unsigned int dns_port);


/**
 * Resolve a DNS name using your custom DNS server.
 *
 * Whenever your proxy needs to open a connection to a web server, it calls
 * resolve() as follows:
 *
 * struct addrinfo *result;
 * int rc = resolve("video.cs.cmu.edu", "8080", null, &result);
 * if (rc != 0) {
 *     // handle error
 * }
 * // connect to address in result
 * free(result);
 *
 *
 * @param  node  The hostname to resolve.
 * @param  service  The desired port number as a string.
 * @param  hints  Should be null. resolve() ignores this parameter.
 * @param  res  The result. resolve() should allocate a struct addrinfo, which
 * the caller is responsible for freeing.
 *
 * @return 0 on success, -1 otherwise
 */

int resolve(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res);


/**
 * Helper function, make dns query packet based on the node param. It's allocated inside, and should be freed by caller
 *
 * @param node The hostname to resolve
 *
 * @return char * to the generated dns query string
 */
char *make_dns_query(const char *node);

/**
 * Helper function, parse reply from dns server.
 *
 * @param dns_reply The reply from dns server
 *
 * @return sockaddr based the parse of dns_reply
 */
struct sockaddr *parse_dns_reply(char *dns_reply);


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
int make_head(char **query, uint16_t msg_id, uint16_t flags, uint16_t QDCOUNT, uint16_t ANCOUNT);

/**
 * Helper function, return the number of entries in the question section
 *
 * @param node The query string
 *
 * @return number of entries of string
 */
uint16_t get_qdcount(const char *node);


#endif
