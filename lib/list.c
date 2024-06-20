#include <depthos/heap.h>
#include <depthos/list.h>

void list_init(struct list *list) {
  list->first = NULL;
  list->last = NULL;
  list->length = 0;
}

// XXX: create list automatically on push

struct list *list_create() {
  struct list *list = (struct list *)kmalloc(sizeof(struct list));
  list_init(list);
  return list;
}

void list_insert(struct list *list, struct list_entry *prev,
                 struct list_entry *next, struct list_entry *entry) {
  if (!entry)
    return;
  entry->prev = prev;
  entry->next = next;
  if (next)
    next->prev = entry;
  else
    list->last = entry;
  if (prev)
    prev->next = entry;
  else
    list->first = entry;

  list->length++;
  return entry;
}

struct list_entry *list_insert_value(struct list *list, struct list_entry *prev,
                                     struct list_entry *next,
                                     list_value_t value) {
  struct list_entry *entry =
      (struct list_entry *)kmalloc(sizeof(struct list_entry));
  entry->value = value;
  list_insert(list, prev, next, entry);
  return entry;
}

void list_remove(struct list *list, struct list_entry *entry) {
  if (!entry)
    return;
  if (!entry->prev) {
    list->first = entry->next;
    if (entry->next)
      entry->next->prev = NULL;
  } else {
    entry->prev->next = entry->next;
    if (entry->next)
      entry->next->prev = entry->prev;
  }

  if (!entry->next)
    list->last = entry->prev;
  list->length--;
}

struct list_entry *list_remove_index(struct list *list, size_t index) {
  size_t i = 0;
  list_foreach(list, entry) {
    if (i == index) {
      list_remove(list, entry);
      return entry;
    }
    i++;
  }
  return NULL;
}

struct list_entry *list_insert_index(struct list *list, size_t index,
                                     list_value_t value) {
  size_t i = 0;
  list_foreach(list, entry) {
    if (i == index) {
      list_insert_value(list, entry, entry->next, value);
      return entry;
    }
  }
  return NULL;
}

struct list_entry *list_push(struct list *list, list_value_t value) {
  return list_insert_value(list, list->last, NULL, value);
}

struct list_entry *list_pop(struct list *list) {
  if (!list->last)
    return NULL;

  struct list_entry *entry = list->last;
  list_remove(list, list->last);
  return entry;
}

struct list_entry *list_push_front(struct list *list, list_value_t value) {
  return list_insert_value(list, NULL, list->first, value);
}

struct list_entry *list_pop_front(struct list *list) {
  if (!list->first)
    return NULL;

  struct list_entry *entry = list->first;
  list_remove(list, list->first);
  return entry;
}
