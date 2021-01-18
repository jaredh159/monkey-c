#include <stdlib.h>
#include <string.h>
#include "./list.h"

List *list_append(List *list, void *item)
{
  List *node = malloc(sizeof(List));
  node->item = item;
  node->next = NULL;

  if (list == NULL)
    return node;

  List *current = list;
  while (current->next != NULL)
    current = current->next;
  current->next = node;

  return list;
}

int list_count(List *list)
{
  int num_items;
  List *current = list;
  for (num_items = 0; current != NULL; num_items++)
    current = current->next;
  return num_items;
}

void list_strcat_each(List *list, char *target_str, char *(*handler)(void *))
{
  List *current = list;
  for (; current != NULL; current = current->next)
    if (current->item != NULL)
      strcat(target_str, handler(current->item));
}
