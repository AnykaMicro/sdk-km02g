/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) 2026 Guangzhou Anyka Microelectronics Co., Ltd. */

/* Host check of ak_list.h: order, removal while walking, double removal. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "ak_list.h"

struct item {
    int v;
    struct ak_list node;
};

int main(void)
{
    struct ak_list head, *n, *t;
    int i, sum = 0, count = 0;

    ak_list_init(&head);
    ak_list_for_each_safe(n, t, &head)
        count++;
    assert(count == 0);

    for (i = 1; i <= 5; i++) {
        struct item *it = malloc(sizeof(*it));
        assert(it != NULL);
        it->v = i;
        ak_list_append(&head, &it->node);
    }

    i = 1;
    ak_list_for_each_safe(n, t, &head) {
        struct item *it = ak_list_item(n, struct item, node);
        assert(it->v == i++);
        if (it->v % 2) {
            ak_list_remove(n);
            ak_list_remove(n);              /* a second removal is harmless */
            free(it);
        }
    }
    ak_list_for_each_safe(n, t, &head)
        sum += ak_list_item(n, struct item, node)->v;
    assert(sum == 2 + 4);

    ak_list_for_each_safe(n, t, &head) {
        ak_list_remove(n);
        free(ak_list_item(n, struct item, node));
    }
    assert(head.next == &head && head.prev == &head);
    puts("list_test: PASS");
    return 0;
}
