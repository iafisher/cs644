#include "cs644.h"
#include "cs644_topfiles.h"

struct topfiles_list topfiles_list_create(size_t capacity) {
  struct topfiles_list lst;
  lst.capacity = capacity;
  lst.len = 0;
  lst.entries = cs644_malloc_or_bail(capacity * sizeof *lst.entries);
  return lst;
}

void topfiles_list_insert_sorted(struct topfiles_list* lst, struct topfiles_entry entry) {
  if (lst->len == 0) {
    lst->entries[0] = entry;
    lst->len = 1;
    return;
  }

  size_t i = 0;
  while (i < lst->len && lst->entries[i].size >= entry.size) {
    i++;
  }

  if (lst->len < lst->capacity) {
    for (size_t j = lst->len; j > i; --j) {
      lst->entries[j] = lst->entries[j - 1];
    }
    lst->entries[i] = entry;
    lst->len++;
  } else {
    if (entry.size > lst->entries[lst->len - 1].size) {
      for (size_t j = lst->len - 1; j > i; --j) {
        lst->entries[j] = lst->entries[j - 1];
      }
      lst->entries[i] = entry;
    }
  }
}

off_t topfiles_list_max_size(struct topfiles_list* lst) {
  return lst->len == 0 ? 0 : lst->entries[lst->len].size;
}
