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
        if (!entry)
                return;

        entry->prev = entry;
        entry->next = entry;
}

static inline _Bool c_list_entry_is_linked(CListEntry *e) {
        return e && e->prev != e;
}

static inline void c_list_prepend(CList *list, CListEntry *entry) {
        assert(!c_list_entry_is_linked(entry));

        if (!list->last) {
                assert(!list->first);
                list->last = entry;
                entry->next = NULL;
        } else {
                assert(list->first);
                list->first->prev = entry;
                entry->next = list->first;
        }

        entry->prev = NULL;
        list->first = entry;
}

static inline void c_list_append(CList *list, CListEntry *entry) {
        assert(!c_list_entry_is_linked(entry));

        if (!list->first) {
                assert(!list->last);
                list->first = entry;
                entry->prev = NULL;
        } else {
                assert(list->last);
                list->last->next = entry;
                entry->prev = list->last;
        }

        entry->next = NULL;
        list->last = entry;
}

static inline CListEntry *c_list_first(CList *list) {
        assert(!list->first == !list->last);

        return list->first;
}

static inline CListEntry *c_list_last(CList *list) {
        assert(!list->first == !list->last);

        return list->last;
}

static inline CListEntry *c_list_entry_prev(CListEntry *entry) {
        if (c_list_entry_is_linked(entry))
                return entry->prev;
        else
                return NULL;
}

static inline CListEntry *c_list_entry_next(CListEntry *entry) {
        if (c_list_entry_is_linked(entry))
                return entry->next;
        else
                return NULL;
}

static inline void c_list_remove(CList *list, CListEntry *entry) {
        if (!c_list_entry_is_linked(entry))
                return;

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

        c_list_entry_init(entry);
}

#ifdef __cplusplus
}
#endif
