#ifndef __LIST_H__
#define __LIST_H__

typedef struct List
{
  void *item;
  struct List *next;
} List;

int list_count(List *list);
List *list_append(List *list, void *item);
void list_strcat_each(List *list, char *target_str, char *(*handler)(void *));

#endif // __LIST_H__
