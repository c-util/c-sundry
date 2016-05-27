#pragma once

/***
  This file is part of c-sundry. See COPYING for details.

  c-sundry is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 2.1 of the License, or
  (at your option) any later version.

  c-sundry is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with c-sundry; If not, see <http://www.gnu.org/licenses/>.
***/

#include <c-macro.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CList CList;
typedef struct CListEntry CListEntry;

struct CList {
        CListEntry *first;
        CListEntry *last;
};

struct CListEntry {
        CListEntry *prev;
        CListEntry *next;
};

static inline void c_list_init(CList *list) {
        if (list)
                *list = (CList){};
}

static inline void c_list_entry_init(CListEntry *entry) {
        if (entry)
                *entry = (CListEntry){};
}

static inline void c_list_prepend(CList *list, CListEntry *entry) {
        assert(!entry->prev);
        assert(!entry->next);

        if (!list->last) {
                assert(!list->first);
                list->last = entry;
        } else {
                assert(list->first);
                list->first->prev = entry;
                entry->next = list->first;
        }

        list->first = entry;
}

static inline void c_list_append(CList *list, CListEntry *entry) {
        assert(!entry->prev);
        assert(!entry->next);

        if (!list->first) {
                assert(!list->last);
                list->first = entry;
        } else {
                assert(list->last);
                list->last->next = entry;
                entry->prev = list->last;
        }

        list->last = entry;
}

static inline void c_list_remove(CList *list, CListEntry *entry) {
        assert(list->first && list->last);

        if (list->first == entry) {
                assert(!entry->prev);
                list->first = entry->next;
        } else {
                assert(entry->prev);
                entry->prev->next = entry->next;
        }

        if (list->last == entry) {
                assert(!entry->next);
                list->last = entry->prev;
        } else {
                assert(entry->next);
                entry->next->prev = entry->prev;
        }

        entry->prev = NULL;
        entry->next = NULL;
}

#ifdef __cplusplus
}
#endif
