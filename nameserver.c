#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "helper.h"
#include "dns_lib.h"
#include "nameserver.h"
#include "graph.h"
#include "list.h"

#ifndef TEST

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
  struct dns_t *query = NULL;
  struct server_t *serverlist = NULL;
  int serverlist_len = 0;
  int reply_len = 0;
  uint32_t next_ip;
  int **graph;
  int graph_size;
  
  if (strcmp(argv[1], "-r") == 0) {
    round_robin = 1;
    log = argv[2];
    ip = argv[3];
    port = atoi(argv[4]);
    servers = argv[5];
    LSAs = argv[6];
  
    serverlist = get_serverlist(servers, &serverlist_len);
  
  } else {
    round_robin = 0;
    log = argv[1];
    ip = argv[2];
    port = atoi(argv[3]);
    servers = argv[4];
    LSAs = argv[5];

    serverlist = get_serverlist(servers, &serverlist_len);
    graph = make_graph(serverlist, LSAs, &graph_size);
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

    
    if (recv_ret == -1) {
      perror("Error! nameserve, recvfrom\n");
      exit(-1);
    }
    
    // udp, a packet is recvd as a whole
    if (recv_ret > 0) {
      dbprintf("nameserver: recved %d bytes\n", recv_ret);

      query = parse_dns(query_buf);
      print_dns(query);
      
      if (round_robin) {

	next_ip = next_server(serverlist, serverlist_len);
	reply_buf = cnd_rr(query, next_ip, &reply_len);


      } else {

	reply_buf = cnd_geo_dist(query, &reply_len, graph, LSAs);
      }

      dbprintf("nameserver: send reply back to proxy\n");
      assert(reply_len != 0);
      if ((send_ret = sendto(sock, reply_buf, reply_len, 0, (struct sockaddr *)&client_addr, client_len)) != reply_len) {
	perror("Error! nameserver, sendto\n");
	exit(-1);
      }
      
      free(reply_buf);
    }

  }
  
  free(query_buf);
}

#endif //TEST

char *cnd_rr(struct dns_t *query, uint32_t ip, int *len) {
  assert(query != NULL);
  assert(ip != 0x00);

  char *reply = NULL;
  
  reply = make_dns_reply(query, ip, len);
  dbprintf("cnd_rr: choose ip %x\n", ip);

  return reply;
}

char *cnd_geo_dist(struct dns_t *query, int *len, int **graph, char *server_list) {
  assert(query != NULL);
  assert(len != NULL);
  
  char *reply = NULL;
  //int server_ind;

  // figout out ip
  //server_ind = dijkstra(graph);

  //reply = make_dns_reply(query, ip, len);
  //dbprintf("cnd_geo_dist: choose ip %x\n", ip);

  //
  printf("Now, just return 15441\n");
  char * ret = (char *)calloc(1024, sizeof(char));
  memcpy(ret, "15441", strlen("15441"));

  return reply;
}

/*
int init_serverlist(struct server_t **list) {
  assert(list != NULL);

  *list = (struct server_t *)calloc(1, sizeof(struct server_t));

  (*list)->server = 0x00;
  (*list)->next = *list;
  
  return 0;
  }*/


uint32_t next_server(struct server_t *list, int list_len) {
  static int ind = 0;

  assert(list != NULL);
  assert(ind <= list_len);
  
  int count = 0;
  
  if (ind == list_len) 
    ind = 0;

  while (count < ind) {
    assert(list != NULL);
    list = list->next;
    ++count;
  }

  ++ind;

  return list->server;
}

struct server_t *get_serverlist(char *servers, int *list_len) {
  assert(servers != NULL);
  assert(list_len != NULL);
  dbprintf("get_server_list:\n");

  //static struct server_t *server_list = NULL;
  FILE *fp = NULL;
  struct server_t *serverlist = NULL;
  int size = 128;
  char line[size];
  struct in_addr tmp;

  //init_serverlist(&serverlist);

  if ((fp = fopen(servers, "r")) == NULL) {
    perror("Error! cnd_rr, fopen\n");
    exit(-1);
  }

  *list_len = 0;
  memset(line, 0, size);
  while (fgets(line, size, fp) != NULL){
    line[strlen(line)-1] = '\0';

    // inet_aton
    memset(&tmp, 0, sizeof(tmp));
    if (inet_aton(line, &tmp) == 0) {
      perror("Error! cnd_rr, inet_aton\n");
      exit(-1);
    }
    
    serverlist = push_server(serverlist, tmp.s_addr, list_len); // uint32_t
    memset(line, 0, size);

    //*list_len += 1;
  }
  
  print_serverlist(serverlist);

  return serverlist;
}



struct server_t *push_server(struct server_t *list, uint32_t server, int *list_len) {
  //assert(list != NULL);

  if (list == NULL) {
    list = (struct server_t *)calloc(1, sizeof(struct server_t));
    list->server = server;
    list->next = NULL;

    *list_len += 1;
    return list;
  }

  struct server_t *p = (struct server_t *)calloc(1, sizeof(struct server_t));
  p->server = server;
  p->next = list;
  list = p;

  *list_len += 1;

  return list;
}

int print_serverlist(struct server_t *list) {
  //assert(list != NULL);
  dbprintf("print_serverlist:\n");
  
  if (list == NULL) {
    printf("list is null\n");
    return 0;
  }

  while (list != NULL) {
    printf("%x, ", list->server);
    list = list->next;
  }
  printf("\n");
  
  return 0;
}


int **make_graph(struct server_t *server_list, char *LSAs, int *graph_size) {
  //assert(server_list != NULL);
  assert(LSAs != NULL);
  //assert(graph_size != NULL);
  
  FILE *fp = NULL;
  int line_size = 1024;
  char line[line_size];
  struct lsa_t *lsa = NULL;
  struct list_node_t *lsa_list = NULL;
  struct list_node_t *ip_list = NULL;

  //init_list(&lsa_list);

  // get host list first
  if((fp = fopen(LSAs, "r")) == NULL) {
    perror("Error! make_graph, fopen");
    exit(-1);
  }

  memset(line, 0, line_size);
  while (fgets(line, line_size, fp) != NULL) {
    lsa = parse_line(line);
    push(&lsa_list, lsa);
  
    collect_ip(&ip_list, lsa);
    
    memset(line, 0, line_size);
  }

  print_list(lsa_list, printer_lsa);

  // to ind list
  
  
  // generate graph
  
  return NULL;

}

char *make_server_list(char *servers) {
  assert(servers != NULL);

  FILE *fp = NULL;
  char *line = NULL;

  if ((fp = fopen(servers, "r")) == NULL) {
    perror("Error! make_server_array, fopen\n");
    exit(-1);
  }
  
  line = (char *)calloc(128, sizeof(char));
  while(fgets(line, 128, fp) != NULL) {
    
  }
  
  return NULL;
}

struct lsa_t *parse_line(char *line) {
  assert(line != NULL);

  char *p1 = NULL;
  char *p2 = NULL;
  struct lsa_t *lsa = NULL;
  struct list_node_t *neighbors = NULL;
  int tmp_size = 128;
  char tmp[tmp_size];
  char *n_ip;

  lsa = (struct lsa_t *)calloc(1, sizeof(struct lsa_t));

  // ip
  p1 = line;
  if ((p2 = strchr(p1, ' ')) == NULL) {
    printf("Error! parse_line, ip, wrong format\n");
    exit(-1);
  }
  
  lsa->ip = (char *)calloc(p2 - p1 + 1, sizeof(char));
  memcpy(lsa->ip, p1, p2-p1);
  //dbprintf("parse_line: ip:%s, ", lsa->ip);

  // seq_num
  p1 = p2 + 1;
  if ((p2 = strchr(p1, ' ')) == NULL) {
    printf("Error! parse_line, seq_num, wrong format\n");
    exit(-1);
  }
  
  memset(tmp, 0, tmp_size);
  memcpy(tmp, p1, p2-p1);
  lsa->seq_num = atoi(tmp);
  //dbprintf("seq_num:%d, ", lsa->seq_num);

  // neighbors  
  //init_list(&neighbors);
  
  //dbprintf("neighbors:");
  p1 = p2 + 1;
  while ((p2 = strchr(p1, ',')) != NULL || (p2 = strchr(p1, '\n')) != NULL) {
    n_ip = (char *)calloc(p2-p1+1, sizeof(char));
    memcpy(n_ip, p1, p2-p1);
    push(&neighbors, n_ip);
    //dbprintf("%s, ", n_ip);
    
    p1 = p2 + 1;
  }

  lsa->neighbors = neighbors;

  // done
  return lsa;
}

void printer_lsa(void *data) {
  assert(data != NULL);
  struct lsa_t *lsa = (struct lsa_t *)data;

  dbprintf("ip:%s, seq_num:%d, neighbors:", lsa->ip, lsa->seq_num);
  print_list(lsa->neighbors, printer_str);

}

int collect_ip(struct list_node_t **ip_list, struct lsa_t *lsa) {
  assert(ip_list != NULL);
  assert(lsa != NULL);
  dbprintf("collect_ip:\n");
  
  char *ip = NULL;
  struct list_node_t *neighbor = NULL;
  char *str = NULL;
  

  // ip part
  if (list_ind(*ip_list, lsa->ip, comparor_str) == -1) {
    ip = (char *)calloc(strlen(lsa->ip)+1, sizeof(char));
    memcpy(ip, lsa->ip, strlen(lsa->ip));
    push(ip_list, ip);
    print_list(*ip_list, printer_str);
  }

  // neighbor list part
  neighbor = lsa->neighbors;
  while (neighbor != NULL) {

    str = (char *)neighbor->data;

    if (list_ind(*ip_list, str, comparor_str) != -1) {
      neighbor = neighbor->next;
      continue;
    }

    ip = (char *)calloc(strlen(str)+1, sizeof(char));
    memcpy(ip, str, strlen(str));

    push(ip_list, ip);
    //print_list(*ip_list, printer_str);

    neighbor = neighbor->next;
  }
  
  return 0;
}


#ifdef TEST

int main(){

  struct server_t *servers = NULL;
  int len = 0;

  //init_serverlist(&servers);
  print_serverlist(servers);
  
  servers = push_server(servers, (uint32_t)0x01, &len);
  print_serverlist(servers);

  servers = push_server(servers, (uint32_t)0x02, &len);
  print_serverlist(servers);

  servers =  push_server(servers, (uint32_t)0x03, &len);
  print_serverlist(servers);
  
  printf("test circularity:\n");
  int count;
  for (count = 0; count < 10; count++) {
    printf("%x, ", next_server(servers, len));
  }
  printf("\n");

  printf("test get_serverlist:\n");
  servers = get_serverlist("./topos/topo1/topo1.servers", &len);
  //print_serverlist(servers);

  // test parse_line
  //struct lsa_t *lsa = parse_line("router2 6 3.0.0.1,4.0.0.1,5.0.0.1,router1\n\n\n");
  //printer_lsa(lsa);

  // test make_graph
  make_graph(NULL, "./topos/topo1/topo1.lsa", NULL);

  return 0;
}

#endif 
