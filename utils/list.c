#include "./list.h"
#include <stdlib.h>
#include <string.h>

List *list_append(List *list, void *item) {
  List *node = malloc(sizeof(List));
  node->item = item;
  node->next = NULL;

  if (list == NULL)
    return node;

  List *current = list;
  while (current->next != NULL) current = current->next;
  current->next = node;

  return list;
}

int list_count(List *list) {
  int num_items;
  List *current = list;
  for (num_items = 0; current != NULL; num_items++) current = current->next;
  return num_items;
}

void list_strcat_each(List *list, char *target_str, char *(*handler)(void *)) {
  List *current = list;
  for (; current != NULL; current = current->next)
    if (current->item != NULL)
      strcat(target_str, handler(current->item));
}

void list_str_join(
  List *list, char *delim, char *target_str, char *(*handler)(void *)) {
  int num_items = list_count(list);

  List *current = list;
  for (int i = 0; i < num_items; i++) {
    strcat(target_str, handler(current->item));
    if (i < (num_items - 1))
      strcat(target_str, delim);
    current = current->next;
  }
}
