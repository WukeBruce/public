#include "list.h"
#include "debug.h"

/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static void __list_add(struct gxlist_head *new_node, struct gxlist_head *head, struct gxlist_head *head_next)
{
	head_next->prev = new_node;
	new_node->next = head_next;
	new_node->prev = head;
	head->next = new_node;
}

/**
 * list_add - add a new entry
 * @new: new entry to be added
 * @head: list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
void gxlist_add(struct gxlist_head *new_node, struct gxlist_head *head)
{
	__list_add(new_node, head, head->next);
}

/**
 * list_add_tail - add a new entry
 * @new: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
void gxlist_add_tail(struct gxlist_head *new_node, struct gxlist_head *head)
{
	__list_add(new_node, head->prev, head);
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static void __list_del(struct gxlist_head *prev_head, struct gxlist_head *next_head)
{
	next_head->prev = prev_head;
	prev_head->next = next_head;
}

/**
 * list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: list_empty on entry does not return true after this, the entry is in an undefined state.
 */
void gxlist_del(struct gxlist_head *entry)
{
	__list_del(entry->prev, entry->next);
}

/**
 * list_del_init - deletes entry from list and reinitialize it.
 * @entry: the element to delete from the list.
 */
void gxlist_del_init(struct gxlist_head *entry)
{
	__list_del(entry->prev, entry->next);
	GX_INIT_LIST_HEAD(entry);
}

/**
 * list_empty - tests whether a list is empty
 * @head: the list to test.
 */
int gxlist_empty(struct gxlist_head *head)
{
	return head->next == head;
}

struct gxlist_head *gxlist_get(struct gxlist_head *head)
{
	struct gxlist_head *first = head->next;

	if (first != head)  {
		__list_del(first->prev, first->next);
		return first;
	}

	return 0;
}

/**
 * list_splice - join two lists
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
void gxlist_splice(struct gxlist_head *list, struct gxlist_head *head)
{
	struct gxlist_head *first = list->next;

	if (first != list) {
		struct gxlist_head *last = list->prev;
		struct gxlist_head *at = head->next;

		first->prev = head;
		head->next = first;

		last->next = at;
		at->prev = last;
	}
}

