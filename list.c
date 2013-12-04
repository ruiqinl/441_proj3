#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "list.h"

/*
int init_list(struct list_node_t **list) {
  assert(list != NULL);
  
  *list = (struct list_node_t *)calloc(1, sizeof(struct list_node_t));
  (*list)->data = NULL;
  (*list)->next = NULL;
  
  return 0;
}
*/

int push(struct list_node_t **list, void *data) {
  assert(list != NULL);
  assert(data != NULL);

  struct list_node_t *p = NULL;
  struct list_node_t *old_node = NULL;
  
  if (*list == NULL) {

    *list = (struct list_node_t *)calloc(1, sizeof(struct list_node_t));
    (*list)->data = data;
    (*list)->next = NULL;

  } else {
    
    p = *list;
    while (p != NULL) {
      old_node = p;
      p = p->next;
    }
    assert(old_node != NULL);
    
    p = (struct list_node_t *)calloc(1, sizeof(struct list_node_t));
    p->data = data;
    p->next = NULL;

    old_node->next = p;
  }

  return 0;
}


struct list_node_t *pop(struct list_node_t **list) {
  assert(list != NULL);

  struct list_node_t *ret = NULL;
  
  if (*list == NULL) {
    printf("Error! pop a empyt list\n");
    exit(-1);
  }
  
  ret = *list;
  *list = (*list)->next;

  return ret;
}

int print_list(struct list_node_t *list, void (*printer)(void *data)) {
  //assert(list != NULL);
  
  if (list == NULL) {
    printf("empty list\n");
    return 0;
  }

  do {
    printer(list->data);
    list = list->next;
  } while (list != NULL);
  printf("\n");
  
  return 0;
}

void printer_int(void *d) {
  int data = *(int*)(d);
  printf("%d, ", data);
}

void printer_str(void *s) {
  char *str = (char *)s;
  printf("%s, ", str);
}

int comparor_str(void *s, void *t) {
  assert(s != NULL);
  assert(t != NULL);

  char *str1 = (char *)s;
  char *str2 = (char *)t;

  return strcmp(str1, str2);
}

int comparor_int(void *a, void *b) {
  //assert(a != NULL);
  //assert(b != NULL); // could be 0, ie null

  int int1 = (int)a;
  int int2 = (int)b;
  
  return int1 - int2;
}

int list_ind(struct list_node_t *list, void *data, int (*comparor)(void *d1, void* d2)) {
  //assert(list != NULL); // needs to support null for collect_ip
  assert(data != NULL);
  
  int ind = 0;

  while (list != NULL) {

    if (comparor(list->data, data) == 0)
      return ind;

    ++ind;

    list = list->next;
  }
  
  if (list == NULL)
    ind = -1;
  
  return ind;
}



#ifdef TEST

int main(){
  struct list_node_t *list = NULL;
  
  //init_list(&list);
  print_list(list, printer_str);

  push(&list, "ab");
  print_list(list, printer_str);
  
  push(&list, "cd");
  print_list(list, printer_str);

  push(&list, "efg");
  print_list(list, printer_str);

  //test list_ind
  printf("ab:%d, cd:%d, efg:%d, xyz:%d\n", list_ind(list, "ab", comparor_str), list_ind(list, "cd", comparor_str), list_ind(list, "efg", comparor_str), list_ind(list, "xyz", comparor_str));
  
  // test pop
  struct list_node_t *p = pop(&list);
  printf("pop:%s\n", (char *)(p->data));
  print_list(list, printer_str);

  p = pop(&list);
  printf("pop:%s\n", (char *)(p->data));
  print_list(list, printer_str);

  p = pop(&list);
  printf("pop:%s\n", (char *)(p->data));
  print_list(list, printer_str);

  
}

#endif
