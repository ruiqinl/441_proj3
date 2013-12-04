#ifndef _NAMESERVER_H_
#define _NAMESERVER_H_

#include "helper.h"
#include "list.h"

struct server_t {
  uint32_t server; // ip
  struct server_t *next;
};

struct lsa_t {
  char *ip;
  int seq_num;
  struct list_node_t *neighbors;
};

/**
 * Choose cnd based on round_roubin, return reply packet
 *
 * @param query The received query
 * @param server_ip The ip to reply
 * @param reply_len Return the length of the reply via param
 *
 * @return reply packet
 */
char *cnd_rr(struct dns_t *query, uint32_t server_ip, int *reply_len);

char *cnd_geo_dist(struct dns_t *query, int *len, int **graph, char *server_list);

struct server_t *get_serverlist(char *servers, int *list_len);
//int init_serverlist(struct server_t **list);
struct server_t *push_server(struct server_t *list, uint32_t ip, int *list_len);
int print_serverlist(struct server_t *list);

uint32_t next_server(struct server_t *list, int list_len);

int **make_graph(struct server_t *server_list, char *LSAs, int *graph_size);
struct lsa_t *parse_line(char *line);
void printer_lsa(void *data);
int collect_ip(struct list_node_t **ip_list, struct lsa_t *lsa);

#endif
