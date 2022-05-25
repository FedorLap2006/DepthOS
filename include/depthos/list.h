#pragma once

#include <depthos/stdtypes.h>

typedef uint64_t list_value_t;

struct list_entry {
  list_value_t value;
  struct list_entry *next;
  struct list_entry *prev;
};

struct list {
  struct list_entry *first;
  struct list_entry *last;
  size_t length;
};

#define list_item(entry, type) ((type)entry->value)

#define list_foreachv(list, item)                                               \
  for (item = list->first; item != NULL; item = item->next)

#define list_foreach(list, item)                                               \
  for (struct list_entry *item = list->first; item != NULL; item = item->next)

void list_init(struct list *list);
struct list *list_create();
void list_insert(struct list *list, struct list_entry *prev,
                 struct list_entry *next, struct list_entry *entry);
struct list_entry *list_insert_value(struct list *list, struct list_entry *prev,
                                     struct list_entry *next,
                                     list_value_t value);
void list_remove(struct list *list, struct list_entry *entry);

struct list_entry *list_remove_index(struct list *list, size_t index);
struct list_entry *list_insert_index(struct list *list, size_t index,
                                     list_value_t value);

struct list_entry *list_push(struct list *list, list_value_t value);
struct list_entry *list_pop(struct list *list);
struct list_entry *list_push_front(struct list *list, list_value_t value);
struct list_entry *list_pop_front(struct list *list);