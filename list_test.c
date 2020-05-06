#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "list_sort.h"

struct mystruct {
  int a;
  struct list_head list;
};

int mycmp(void* priv, struct list_head* a, struct list_head* b) {
  struct mystruct* pa = list_entry(a, struct mystruct, list);
  struct mystruct* pb = list_entry(b, struct mystruct, list);
  return pa->a > pb->a;
}

int main() {
  int i = 0;
  struct list_head head = LIST_HEAD_INIT(head);
  struct mystruct* p;
  srand(time(NULL));
  for (i = 0; i < 10; i++) {
    p = (struct mystruct*)malloc(sizeof(struct mystruct));
    p->a = rand() % 100;
    list_add_tail(&p->list, &head);
  }

  struct mystruct* p3;
  int c = 0;
  list_for_each_entry(p, &head, struct mystruct, list) {
    printf("%d,", p->a);
    if (c++ == 3) p3 = p;
  }
  printf("\n");

  list_del_init(&p3->list);

  list_for_each_entry(p, &head, struct mystruct, list) {
    printf("%d,", p->a);
    if (p->a == 3) p3 = p;
  }
  printf("\n");

  list_sort(NULL, &head, mycmp);

  list_for_each_entry(p, &head, struct mystruct, list) {
    printf("%d,", p->a);
    if (p->a == 3) p3 = p;
  }
  printf("\n");
}
