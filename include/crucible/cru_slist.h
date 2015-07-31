// Copyright 2015 Intel Corporation
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice (including the next
// paragraph) shall be included in all copies or substantial portions of the
// Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

#pragma once

#include <stdatomic.h>
#include <stdbool.h>

#include <crucible/xalloc.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cru_slist cru_slist_t;

/// \brief A singly linked non-intrusive list.
struct cru_slist {
    void *data;
    cru_slist_t *next;
};

static inline size_t
cru_slist_length(cru_slist_t *list)
{
    size_t n = 0;

    while (list) {
        ++n;
        list = list->next;
    }

    return n;
}

static inline void
cru_slist_prepend(cru_slist_t **list, void *data)
{
    cru_slist_t *elem;

    elem = xmalloc(sizeof(*elem));
    elem->data = data;

    elem->next = *list;
    *list = elem;
}

static inline void
cru_slist_prepend_atomic(cru_slist_t **list, void *data)
{
    cru_slist_t *elem;

    elem = xmalloc(sizeof(*elem));
    elem->data = data;
    elem->next = atomic_load(list);

    while (!atomic_compare_exchange_strong(list, &elem->next, elem)) {}
}

/// Pop off the list's first node and return the node's data.
///
/// Not threadsafe. Return NULL if the list is empty or if the node has no
/// data.
static inline void *
cru_slist_pop(cru_slist_t **list)
{
    cru_slist_t *old_list;
    void *data;

    if (!*list)
        return NULL;

    old_list = *list;
    *list = old_list->next;
    data = old_list->data;
    free(old_list);

    return data;
}

#ifdef __cplusplus
}
#endif
