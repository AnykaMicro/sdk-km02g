/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) 2026 Guangzhou Anyka Microelectronics Co., Ltd. */

/* Intrusive circular doubly linked list: embed a struct ak_list in your item. */

#ifndef AK_LIST_H
#define AK_LIST_H

#include <stddef.h>

struct ak_list {
    struct ak_list *prev;
    struct ak_list *next;
};

/* An empty list (or an unlinked node) points at itself. */
static inline void ak_list_init(struct ak_list *head)
{
    head->prev = head;
    head->next = head;
}

/* Insert node at the tail of the list. */
static inline void ak_list_append(struct ak_list *head, struct ak_list *node)
{
    node->prev = head->prev;
    node->next = head;
    head->prev->next = node;
    head->prev = node;
}

/* Unlink node and leave it self-linked, so removing it twice is harmless. */
static inline void ak_list_remove(struct ak_list *node)
{
    node->prev->next = node->next;
    node->next->prev = node->prev;
    ak_list_init(node);
}

/* The item that embeds node as its member field. */
#define ak_list_item(node, type, member) \
    ((type *)((char *)(node) - offsetof(type, member)))

/* Visit every node; the current node may be removed or freed in the body. */
#define ak_list_for_each_safe(node, tmp, head) \
    for ((node) = (head)->next, (tmp) = (node)->next; \
         (node) != (head); \
         (node) = (tmp), (tmp) = (node)->next)

#endif /* AK_LIST_H */
