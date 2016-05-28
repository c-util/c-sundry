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

/*
 * Tests for <c-list.h>
 * Bunch of tests for all functionality exported by c-list.h.
 */

#undef NDEBUG
#include <c-macro.h>
#include <c-list.h>

/* test c_list_* helpers */
static void test_list(void) {
        CList list;
        CListEntry entries[4];

        c_list_init(&list);
        for (unsigned int i = 0; i < 4; ++i)
                c_list_entry_init(&entries[i]);

        c_list_append(&list, &entries[2]);
        assert(list.first == &entries[2]);
        assert(list.last == &entries[2]);
        c_list_remove(&list, &entries[2]);
        assert(!list.first);
        assert(!list.last);

        c_list_prepend(&list, &entries[2]);
        assert(list.first == &entries[2]);
        assert(list.last == &entries[2]);
        c_list_append(&list, &entries[3]);
        assert(list.first == &entries[2]);
        assert(list.last == &entries[3]);
        assert(entries[3].prev == &entries[2]);
        assert(entries[2].next == &entries[3]);
        c_list_prepend(&list, &entries[1]);
        assert(list.first == &entries[1]);
        assert(list.last == &entries[3]);
        assert(entries[2].prev == &entries[1]);
        assert(entries[1].next == &entries[2]);
        c_list_prepend(&list, &entries[0]);
        assert(list.first == &entries[0]);
        assert(list.last == &entries[3]);
        assert(entries[1].prev == &entries[0]);
        assert(entries[0].next == &entries[1]);

        c_list_remove(&list, &entries[1]);
        c_list_remove(&list, &entries[2]);
        c_list_remove(&list, &entries[3]);
        c_list_remove(&list, &entries[0]);

        assert(!list.first && !list.last);
        for (unsigned int i = 0; i < 4; ++i)
                assert(entries[i].prev == &entries[i] &&
                       entries[i].next == &entries[i]);
}

int main(int argc, char **argv) {
        test_list();
        return 0;
}
