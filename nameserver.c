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
  struct list_node_t *lsa_list = NULL;
  struct list_node_t *ip_list = NULL;

  int client_ind, server_ind;
  struct list_node_t *server_ind_list = NULL;
  
  
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

    graph = make_graph(LSAs, &graph_size, &lsa_list, &ip_list);
    serverlist = get_serverlist(servers, &serverlist_len);
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
	dbprintf("nameserver: round_rodin\n");
	assert(serverlist != NULL);

	next_ip = next_server(serverlist, serverlist_len);
	reply_buf = cnd_rr(query, next_ip, &reply_len);

      } else {
	dbprintf("nameserver: geo_dist\n");
	assert(graph != NULL);
	assert(graph_size > 0);
	assert(lsa_list != NULL);
	assert(ip_list != NULL);
	assert(serverlist != NULL);
	
	client_ind = get_client_ind(&client_addr, ip_list);
	server_ind_list = get_server_ind(serverlist, ip_list);

	server_ind = do_dijkstra(graph, graph_size, client_ind, server_ind_list);
	
	//reply_buf = cnd_geo_dist(query, &reply_len, graph, graph_size);
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


int get_client_ind(struct sockaddr_in *client_addr, struct list_node_t *ip_list) {
  assert(client_addr != NULL);
  assert(ip_list != NULL);

  char *client_ip = NULL;
  int client_ind;

  if ((client_ip = inet_ntoa(client_addr->sin_addr)) == NULL) {
    perror("Error! get_clietn_ind, inet_ntoa");
    exit(-1);
  }
  
  client_ind = list_ind(ip_list, client_ip, comparor_str);
  assert(client_ind != -1);

  printf("get_client_ind:%s, %d\n", client_ip, client_ind);

  return client_ind;
}

struct list_node_t *get_server_ind(struct server_t *serverlist, struct list_node_t *ip_list) {
  
  printf("get_server_ind: not imp yet\n");
  return NULL;
}


uint32_t do_dijkstra(int **graph, int graph_size, int client, struct list_node_t *servers) {

  printf("do_dijk: not imp yet\n");
  return 0;
}

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


int **make_graph(char *LSAs, int *graph_size, struct list_node_t **ret_lsa_list, struct list_node_t **ret_ip_list) {
  assert(LSAs != NULL);
  assert(graph_size != NULL);
  assert(ret_lsa_list != NULL);
  assert(ret_ip_list != NULL);
  
  struct list_node_t *lsa_list = NULL; //
  struct list_node_t *ip_list = NULL; //
  int **matrix = NULL;

  // get adj_list and ip_list
  get_graph_list(&lsa_list, &ip_list, LSAs);
  *ret_ip_list = ip_list;
  *ret_lsa_list = lsa_list;

  //print_list(lsa_list, printer_lsa);
  //print_list(ip_list, printer_str);
  
  // get adj_matrix
  matrix = get_adj_matrix(lsa_list, ip_list, graph_size);

  
  return matrix;

}

int **get_adj_matrix(struct list_node_t *lsa_list, struct list_node_t *ip_list, int *matrix_size) {
  assert(lsa_list != NULL);
  assert(ip_list != NULL);
  assert(list_size != NULL);
  printf("get_adj_matrix:\n");

  int size;
  int **matrix = NULL;
  int i, j;
  struct list_node_t *tmp_node = NULL;
  char *tmp_ip = NULL;
  struct lsa_t *tmp_lsa = NULL;
  int ind;

  // size
  size = list_size(ip_list);
  *matrix_size = size;

  // calloc space, and init to -1 which indicating no edge
  matrix = (int **)calloc(size, sizeof(int *));

  for (i = 0; i < size; i++) {
    matrix[i] = (int *)calloc(size, sizeof(int));
    
    for (j = 0; j < size; j++) {
      if (i != j)
	matrix[i][j] = -1;
      else 
	matrix[i][j] = 0; // diag line is 0 line
    }
  }

  // fill in matrix, each ip is natually assigned index in the ip_list as id
  for (i = 0; i < size; i++) {

    // find lsa with index i
    tmp_node = list_node(ip_list, i);
    tmp_ip = (char *)(tmp_node->data);
    //dbprintf("%s with ind_%d\n", tmp_ip, i);

    ind = list_ind(lsa_list, tmp_ip, comparor_lsa_ip);
    if (ind == -1) {
      printf("Warning! %s_%d does not exist in nei_list\n", tmp_ip, i);
      continue;
    }
    tmp_node = list_node(lsa_list, ind);
    tmp_lsa = (struct lsa_t *)(tmp_node->data);
    //dbprintf("with lsa:");
    //printer_lsa(tmp_lsa);

    // modify adj_matrix
    set_adj_line(matrix, i, tmp_lsa, ip_list);
  }

 
  return matrix;
}


int set_adj_line(int **matrix, int line_ind, struct lsa_t *lsa, struct list_node_t *ip_list) {
  assert(matrix != NULL);
  assert(lsa != NULL);
  assert(ip_list != NULL);

  struct list_node_t *nei = NULL;
  char *nei_ip = NULL;
  int nei_ind;
  
  nei = lsa->neighbors;

  while (nei != NULL) {
    nei_ip = (char *)nei->data;
    nei_ind = list_ind(ip_list, nei_ip, comparor_str);
    assert(nei_ind != -1);

    matrix[line_ind][nei_ind] = 1;

    //dbprintf("matrix[%d][%d] = 1, ", line_ind, nei_ind);
    
    nei = nei->next;
  }
  //dbprintf("\n");

  return 0;
}


int comparor_lsa_ip(void *lsa_void, void *ip_void) {
  assert(lsa_void != NULL);
  assert(ip_void != NULL);

  struct lsa_t *lsa = (struct lsa_t *)lsa_void;
  char *ip = (char *)ip_void;

  return strcmp(lsa->ip, ip);

}


int get_graph_list(struct list_node_t **ret_nei_list, struct list_node_t **ret_ip_list, char *LSAs) {

  assert(LSAs != NULL);
  assert(ret_nei_list != NULL);
  assert(ret_ip_list != NULL);

  FILE *fp = NULL;
  int line_size = 1024;
  char line[line_size];
  struct lsa_t *lsa = NULL;
  struct list_node_t *lsa_list = NULL; //
  struct list_node_t *ip_list = NULL; //
  int ind;
  struct list_node_t *tmp_node = NULL;
  struct lsa_t *tmp_lsa = NULL;

  //init_list(&lsa_list);


  // get neighbor list and ip list first
  if((fp = fopen(LSAs, "r")) == NULL) {
    perror("Error! make_graph, fopen");
    exit(-1);
  }

  memset(line, 0, line_size);
  while (fgets(line, line_size, fp) != NULL) {
    lsa = parse_line(line);
    //dbprinter_lsa(lsa);//

    // ip_list
    collect_ip(&ip_list, lsa);

    // lsa_list
    if ((ind = list_ind(lsa_list, lsa, comparor_lsa)) != -1) {
      //dbprintf("already exist at %d", ind);// 
      tmp_node = list_node(lsa_list, ind);
      tmp_lsa = (struct lsa_t *)(tmp_node->data);
      
      //dbprintf("compare with old lsa\n");//
      //printer_lsa(tmp_lsa);
      
      //dbprintf("new %d ? old %d\n", tmp_lsa->seq_num, lsa->seq_num);
      if (tmp_lsa->seq_num < lsa->seq_num)
	tmp_node->data = lsa;

    } else {
      push(&lsa_list, lsa);
    }     
    
    memset(line, 0, line_size);
  }

  //print_list(lsa_list, printer_lsa);

  // ret lists
  *ret_nei_list = lsa_list;
  *ret_ip_list = ip_list;
 
  return 0;
}

int comparor_lsa(void *lsa1, void *lsa2) {
  assert(lsa1 != NULL);
  assert(lsa2 != NULL);
  
  struct lsa_t *a = (struct lsa_t *)lsa1;
  struct lsa_t *b = (struct lsa_t *)lsa2;

  return strcmp(a->ip, b->ip);
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
  //dbprintf("collect_ip:\n");
  
  char *ip = NULL;
  struct list_node_t *neighbor = NULL;
  char *str = NULL;
  

  // ip part
  if (list_ind(*ip_list, lsa->ip, comparor_str) == -1) {
    ip = (char *)calloc(strlen(lsa->ip)+1, sizeof(char));
    memcpy(ip, lsa->ip, strlen(lsa->ip));
    push(ip_list, ip);

    //print_list(*ip_list, printer_str);//
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

    neighbor = neighbor->next;

    //print_list(*ip_list, printer_str);//
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

  //test get_graph_list
  
  //struct list_node_t *lsa_list = NULL; //
  //struct list_node_t *ip_list = NULL; //

  //get_graph_list(&lsa_list, &ip_list, "./topos/topo2/topo2.lsa");

  //print_list(lsa_list, printer_lsa);
  //print_list(ip_list, printer_str);


  // test make_graph
  int size;
  struct list_node_t *lsa_list = NULL;
  struct list_node_t *ip_list = NULL;
  int **matrix = NULL;

  matrix = make_graph("./topos/topo1/topo1.lsa", &size, &lsa_list, &ip_list);

  print_list(lsa_list, printer_lsa);
  print_list(ip_list, printer_str);
  
  int i, j;
  for (i = 0; i < size; i++) {
    for (j = 0; j < size; j++)
      printf("%2d, ", matrix[i][j]);
    printf("\n");
  }

  return 0;
}

#endif 
