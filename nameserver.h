#ifndef _NAMESERVER_H_
#define _NAMESERVER_H_

/**
 * Parse the received query, choose cnd, return reply packet
 *
 * @param query The received query
 *
 * @return reply packet
 */
char *choose_cnd(char *query);


#endif
