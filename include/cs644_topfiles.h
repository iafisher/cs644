#ifndef CS644_TOPFILES_H
#define CS644_TOPFILES_H

#include <stddef.h>

struct topfiles_entry {
  const char* name;
  off_t size;
};

struct topfiles_list {
  struct topfiles_entry* entries;
  size_t len, capacity;
};

struct topfiles_list topfiles_list_create(size_t capacity);
void topfiles_list_insert_sorted(struct topfiles_list* lst, struct topfiles_entry entry);
off_t topfiles_list_max_size(struct topfiles_list* lst);

#endif // CS644_TOPFILES_H
