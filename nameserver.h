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

int **make_graph(char *LSAs, int *graph_size, struct list_node_t **ret_lsa_list, struct list_node_t **ret_ip_list);
struct lsa_t *parse_line(char *line);
void printer_lsa(void *data);
int collect_ip(struct list_node_t **ip_list, struct lsa_t *lsa);
int comparor_lsa(void *lsa1, void *lsa2);

int get_graph_list(struct list_node_t **ret_nei_list, struct list_node_t **ret_ip_list, char *LSAs);

int **get_adj_matrix(struct list_node_t *lsa_list, struct list_node_t *ip_list, int *list_size);

int comparor_lsa_ip(void *lsa_void, void *ip_void);
int set_adj_line(int **matrix, int line_ind, struct lsa_t *lsa, struct list_node_t *ip_list);

#endif
