#ifndef __LIST_H__
#define __LIST_H__

typedef struct List
{
  void *item;
  struct List *next;
} List;

int list_count(List *list);
List *list_append(List *list, void *item);

#endif // __LIST_H__
