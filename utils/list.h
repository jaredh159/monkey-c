#ifndef __LIST_H__
#define __LIST_H__

typedef struct List {
  void *item;
  struct List *next;
} List;

typedef char *(*StrHandler)(void *);

int list_count(List *list);
List *list_append(List *list, void *item);
void list_strcat_each(List *list, char *target_str, StrHandler handler);
void list_str_join(
  List *list, char *delim, char *target_str, StrHandler handler);

#endif  // __LIST_H__
