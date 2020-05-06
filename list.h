#ifndef LIST_H_20200320
#define LIST_H_20200320
#include <stdint.h>
#include <stdlib.h>
/*
 * Simple doubly linked list implementation.
 *
 * Some of the internal functions ("__xxx") are useful when
 * manipulating whole lists rather than single entries, as
 * sometimes we already know the next/prev entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 */

#define LIST_POISON1 ((void *)0x00100100)
#define LIST_POISON2 ((void *)0x00200200)

struct list_head {
  struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(name) \
  { &(name), &(name) }

#define LIST_HEAD(name) struct list_head name = LIST_HEAD_INIT(name)

/**
 * INIT_LIST_HEAD - Initialize a list_head structure
 * @list: list_head structure to be initialized.
 *
 * Initializes the list_head to point to itself.  If it is a list header,
 * the result is an empty list.
 */
static void INIT_LIST_HEAD(struct list_head *list) {
  list->next = list;
  list->prev = list;
}

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:        the pointer to the member.
 * @type:       the type of the container struct this is embedded in.
 * @member:     the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) \
  ((type *)((char *)(ptr) - ((size_t) & ((type *)0)->member)))

/*
 * Insert a new_node entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static void __list_add(struct list_head *new_node, struct list_head *prev,
                       struct list_head *next) {
  next->prev = new_node;
  new_node->next = next;
  new_node->prev = prev;
  prev->next = new_node;
}

/**
 * list_add - add a new_node entry
 * @new_node: new_node entry to be added
 * @head: list head to add it after
 *
 * Insert a new_node entry after the specified head.
 * This is good for implementing stacks.
 */
static void list_add(struct list_head *new_node, struct list_head *head) {
  __list_add(new_node, head, head->next);
}

/**
 * list_add_tail - add a new_node entry
 * @new_node: new_node entry to be added
 * @head: list head to add it before
 *
 * Insert a new_node entry before the specified head.
 * This is useful for implementing queues.
 */
static void list_add_tail(struct list_head *new_node, struct list_head *head) {
  __list_add(new_node, head->prev, head);
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static void __list_del(struct list_head *prev, struct list_head *next) {
  next->prev = prev;
  prev->next = next;
}

/*
 * Delete a list entry and clear the 'prev' pointer.
 *
 * This is a special-purpose list clearing method used in the networking code
 * for lists allocated as per-cpu, where we don't want to incur the extra
 * ) overhead of a regular list_del_init(). The code that uses this
 * needs to check the node 'prev' pointer instead of calling list_empty().
 */
static void __list_del_clearprev(struct list_head *entry) {
  __list_del(entry->prev, entry->next);
  entry->prev = NULL;
}

static void __list_del_entry(struct list_head *entry) {
  __list_del(entry->prev, entry->next);
}

/**
 * list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: list_empty() on entry does not return true after this, the entry is
 * in an undefined state.
 */
static void list_del(struct list_head *entry) {
  __list_del_entry(entry);
  entry->next = (struct list_head *)LIST_POISON1;
  entry->prev = (struct list_head *)LIST_POISON2;
}

/**
 * list_replace - replace old entry by new_node one
 * @old : the element to be replaced
 * @new_node : the new_node element to insert
 *
 * If @old was empty, it will be overwritten.
 */
static void list_replace(struct list_head *old, struct list_head *new_node) {
  new_node->next = old->next;
  new_node->next->prev = new_node;
  new_node->prev = old->prev;
  new_node->prev->next = new_node;
}

/**
 * list_replace_init - replace old entry by new_node one and initialize the old
 * one
 * @old : the element to be replaced
 * @new_node : the new_node element to insert
 *
 * If @old was empty, it will be overwritten.
 */
static void list_replace_init(struct list_head *old,
                              struct list_head *new_node) {
  list_replace(old, new_node);
  INIT_LIST_HEAD(old);
}

/**
 * list_swap - replace entry1 with entry2 and re-add entry1 at entry2's position
 * @entry1: the location to place entry2
 * @entry2: the location to place entry1
 */
static void list_swap(struct list_head *entry1, struct list_head *entry2) {
  struct list_head *pos = entry2->prev;

  list_del(entry2);
  list_replace(entry1, entry2);
  if (pos == entry1) pos = entry2;
  list_add(entry1, pos);
}

/**
 * list_del_init - deletes entry from list and reinitialize it.
 * @entry: the element to delete from the list.
 */
static void list_del_init(struct list_head *entry) {
  __list_del_entry(entry);
  INIT_LIST_HEAD(entry);
}

/**
 * list_move - delete from one list and add as another's head
 * @list: the entry to move
 * @head: the head that will precede our entry
 */
static void list_move(struct list_head *list, struct list_head *head) {
  __list_del_entry(list);
  list_add(list, head);
}

/**
 * list_move_tail - delete from one list and add as another's tail
 * @list: the entry to move
 * @head: the head that will follow our entry
 */
static void list_move_tail(struct list_head *list, struct list_head *head) {
  __list_del_entry(list);
  list_add_tail(list, head);
}

/**
 * list_bulk_move_tail - move a subsection of a list to its tail
 * @head: the head that will follow our entry
 * @first: first entry to move
 * @last: last entry to move, can be the same as first
 *
 * Move all entries between @first and including @last before @head.
 * All three entries must belong to the same linked list.
 */
static void list_bulk_move_tail(struct list_head *head, struct list_head *first,
                                struct list_head *last) {
  first->prev->next = last->next;
  last->next->prev = first->prev;

  head->prev->next = first;
  first->prev = head->prev;

  last->next = head;
  head->prev = last;
}

/**
 * list_is_first -- tests whether @list is the first entry in list @head
 * @list: the entry to test
 * @head: the head of the list
 */
static int list_is_first(const struct list_head *list,
                         const struct list_head *head) {
  return list->prev == head;
}

/**
 * list_is_last - tests whether @list is the last entry in list @head
 * @list: the entry to test
 * @head: the head of the list
 */
static int list_is_last(const struct list_head *list,
                        const struct list_head *head) {
  return list->next == head;
}

/**
 * list_empty - tests whether a list is empty
 * @head: the list to test.
 */
static int list_empty(const struct list_head *head) {
  return (head->next) == head;
}

/**
 * list_empty_careful - tests whether a list is empty and not being modified
 * @head: the list to test
 *
 * Description:
 * tests whether a list is empty _and_ checks that no other CPU might be
 * in the process of modifying either member (next or prev)
 *
 * NOTE: using list_empty_careful() without synchronization
 * can only be safe if the only activity that can happen
 * to the list entry is list_del_init(). Eg. it cannot be used
 * if another CPU could re-list_add() it.
 */
static int list_empty_careful(const struct list_head *head) {
  struct list_head *next = head->next;
  return (next == head) && (next == head->prev);
}

/**
 * list_rotate_left - rotate the list to the left
 * @head: the head of the list
 */
static void list_rotate_left(struct list_head *head) {
  struct list_head *first;

  if (!list_empty(head)) {
    first = head->next;
    list_move_tail(first, head);
  }
}

/**
 * list_rotate_to_front() - Rotate list to specific item.
 * @list: The desired new_node front of the list.
 * @head: The head of the list.
 *
 * Rotates list so that @list becomes the new_node front of the list.
 */
static void list_rotate_to_front(struct list_head *list,
                                 struct list_head *head) {
  /*
   * Deletes the list head from the list denoted by @head and
   * places it as the tail of @list, this effectively rotates the
   * list so that @list is at the front.
   */
  list_move_tail(head, list);
}

/**
 * list_is_singular - tests whether a list has just one entry.
 * @head: the list to test.
 */
static int list_is_singular(const struct list_head *head) {
  return !list_empty(head) && (head->next == head->prev);
}

static void __list_cut_position(struct list_head *list, struct list_head *head,
                                struct list_head *entry) {
  struct list_head *new_first = entry->next;
  list->next = head->next;
  list->next->prev = list;
  list->prev = entry;
  entry->next = list;
  head->next = new_first;
  new_first->prev = head;
}

/**
 * list_cut_position - cut a list into two
 * @list: a new_node list to add all removed entries
 * @head: a list with entries
 * @entry: an entry within head, could be the head itself
 *    and if so we won't cut the list
 *
 * This helper moves the initial part of @head, up to and
 * including @entry, from @head to @list. You should
 * pass on @entry an element you know is on @head. @list
 * should be an empty list or a list you do not care about
 * losing its data.
 *
 */
static void list_cut_position(struct list_head *list, struct list_head *head,
                              struct list_head *entry) {
  if (list_empty(head)) return;
  if (list_is_singular(head) && (head->next != entry && head != entry)) return;
  if (entry == head)
    INIT_LIST_HEAD(list);
  else
    __list_cut_position(list, head, entry);
}

/**
 * list_cut_before - cut a list into two, before given entry
 * @list: a new_node list to add all removed entries
 * @head: a list with entries
 * @entry: an entry within head, could be the head itself
 *
 * This helper moves the initial part of @head, up to but
 * excluding @entry, from @head to @list.  You should pass
 * in @entry an element you know is on @head.  @list should
 * be an empty list or a list you do not care about losing
 * its data.
 * If @entry == @head, all entries on @head are moved to
 * @list.
 */
static void list_cut_before(struct list_head *list, struct list_head *head,
                            struct list_head *entry) {
  if (head->next == entry) {
    INIT_LIST_HEAD(list);
    return;
  }
  list->next = head->next;
  list->next->prev = list;
  list->prev = entry->prev;
  list->prev->next = list;
  head->next = entry;
  entry->prev = head;
}

static void __list_splice(const struct list_head *list, struct list_head *prev,
                          struct list_head *next) {
  struct list_head *first = list->next;
  struct list_head *last = list->prev;

  first->prev = prev;
  prev->next = first;

  last->next = next;
  next->prev = last;
}

/**
 * list_splice - join two lists, this is designed for stacks
 * @list: the new_node list to add.
 * @head: the place to add it in the first list.
 */
static void list_splice(const struct list_head *list, struct list_head *head) {
  if (!list_empty(list)) __list_splice(list, head, head->next);
}

/**
 * list_splice_tail - join two lists, each list being a queue
 * @list: the new_node list to add.
 * @head: the place to add it in the first list.
 */
static void list_splice_tail(struct list_head *list, struct list_head *head) {
  if (!list_empty(list)) __list_splice(list, head->prev, head);
}

/**
 * list_splice_init - join two lists and reinitialise the emptied list.
 * @list: the new_node list to add.
 * @head: the place to add it in the first list.
 *
 * The list at @list is reinitialised
 */
static void list_splice_init(struct list_head *list, struct list_head *head) {
  if (!list_empty(list)) {
    __list_splice(list, head, head->next);
    INIT_LIST_HEAD(list);
  }
}

/**
 * list_splice_tail_init - join two lists and reinitialise the emptied list
 * @list: the new_node list to add.
 * @head: the place to add it in the first list.
 *
 * Each of the lists is a queue.
 * The list at @list is reinitialised
 */
static void list_splice_tail_init(struct list_head *list,
                                  struct list_head *head) {
  if (!list_empty(list)) {
    __list_splice(list, head->prev, head);
    INIT_LIST_HEAD(list);
  }
}

/**
 * list_entry - get the struct for this entry
 * @ptr:    the &struct list_head pointer.
 * @type:    the type of the struct this is embedded in.
 * @member:    the name of the list_head within the struct.
 */
#define list_entry(ptr, type, member) container_of(ptr, type, member)

/**
 * list_first_entry - get the first element from a list
 * @ptr:    the list head to take the element from.
 * @type:    the type of the struct this is embedded in.
 * @member:    the name of the list_head within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define list_first_entry(ptr, type, member) \
  list_entry((ptr)->next, type, member)

/**
 * list_last_entry - get the last element from a list
 * @ptr:    the list head to take the element from.
 * @type:    the type of the struct this is embedded in.
 * @member:    the name of the list_head within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define list_last_entry(ptr, type, member) list_entry((ptr)->prev, type, member)

/**
 * list_first_entry_or_null - get the first element from a list
 * @ptr:    the list head to take the element from.
 * @type:    the type of the struct this is embedded in.
 * @member:    the name of the list_head within the struct.
 *
 * Note that if the list is empty, it returns NULL.
 */
#define list_first_entry_or_null(ptr, type, member)           \
  ({                                                          \
    struct list_head *head__ = (ptr);                         \
    struct list_head *pos__ = (head__->next);                 \
    pos__ != head__ ? list_entry(pos__, type, member) : NULL; \
  })

/**
 * list_next_entry - get the next element in list
 * @pos:    the type * to cursor
 * @member:    the name of the list_head within the struct.
 */
#define list_next_entry(pos, type, member) \
  list_entry((pos)->member.next, type, member)

/**
 * list_prev_entry - get the prev element in list
 * @pos:    the type * to cursor
 * @member:    the name of the list_head within the struct.
 */
#define list_prev_entry(pos, type, member) \
  list_entry((pos)->member.prev, type, member)

/**
 * list_for_each    -    iterate over a list
 * @pos:    the &struct list_head to use as a loop cursor.
 * @head:    the head for your list.
 */
#define list_for_each(pos, head) \
  for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * list_for_each_continue - continue iteration over a list
 * @pos:    the &struct list_head to use as a loop cursor.
 * @head:    the head for your list.
 *
 * Continue to iterate over a list, continuing after the current position.
 */
#define list_for_each_continue(pos, head) \
  for (pos = pos->next; pos != (head); pos = pos->next)

/**
 * list_for_each_prev    -    iterate over a list backwards
 * @pos:    the &struct list_head to use as a loop cursor.
 * @head:    the head for your list.
 */
#define list_for_each_prev(pos, head) \
  for (pos = (head)->prev; pos != (head); pos = pos->prev)

/**
 * list_for_each_safe - iterate over a list safe against removal of list entry
 * @pos:    the &struct list_head to use as a loop cursor.
 * @n:        another &struct list_head to use as temporary storage
 * @head:    the head for your list.
 */
#define list_for_each_safe(pos, n, head) \
  for (pos = (head)->next, n = pos->next; pos != (head); pos = n, n = pos->next)

/**
 * list_for_each_prev_safe - iterate over a list backwards safe against removal
 * of list entry
 * @pos:    the &struct list_head to use as a loop cursor.
 * @n:        another &struct list_head to use as temporary storage
 * @head:    the head for your list.
 */
#define list_for_each_prev_safe(pos, n, head) \
  for (pos = (head)->prev, n = pos->prev; pos != (head); pos = n, n = pos->prev)

/**
 * list_for_each_entry    -    iterate over list of given type
 * @pos:    the type * to use as a loop cursor.
 * @head:    the head for your list.
 * @member:    the name of the list_head within the struct.
 */
#define list_for_each_entry(pos, head, type, member)                       \
  for (pos = list_first_entry(head, type, member); &pos->member != (head); \
       pos = list_next_entry(pos, type, member))

/**
 * list_for_each_entry_reverse - iterate backwards over list of given type.
 * @pos:    the type * to use as a loop cursor.
 * @head:    the head for your list.
 * @member:    the name of the list_head within the struct.
 */
#define list_for_each_entry_reverse(pos, head, type, member)              \
  for (pos = list_last_entry(head, type, member); &pos->member != (head); \
       pos = list_prev_entry(pos, type, member))

/**
 * list_prepare_entry - prepare a pos entry for use in
 * list_for_each_entry_continue()
 * @pos:    the type * to use as a start point
 * @head:    the head of the list
 * @member:    the name of the list_head within the struct.
 *
 * Prepares a pos entry for use as a start point in
 * list_for_each_entry_continue().
 */
#define list_prepare_entry(pos, head, type, member) \
  ((pos) ?: list_entry(head, type, member))

/**
 * list_for_each_entry_continue - continue iteration over list of given type
 * @pos:    the type * to use as a loop cursor.
 * @head:    the head for your list.
 * @member:    the name of the list_head within the struct.
 *
 * Continue to iterate over list of given type, continuing after
 * the current position.
 */
#define list_for_each_entry_continue(pos, head, type, member)            \
  for (pos = list_next_entry(pos, type, member); &pos->member != (head); \
       pos = list_next_entry(pos, type, member))

/**
 * list_for_each_entry_continue_reverse - iterate backwards from the given point
 * @pos:    the type * to use as a loop cursor.
 * @head:    the head for your list.
 * @member:    the name of the list_head within the struct.
 *
 * Start to iterate over list of given type backwards, continuing after
 * the current position.
 */
#define list_for_each_entry_continue_reverse(pos, head, type, member)    \
  for (pos = list_prev_entry(pos, type, member); &pos->member != (head); \
       pos = list_prev_entry(pos, type, member))

/**
 * list_for_each_entry_from - iterate over list of given type from the current
 * point
 * @pos:    the type * to use as a loop cursor.
 * @head:    the head for your list.
 * @member:    the name of the list_head within the struct.
 *
 * Iterate over list of given type, continuing from current position.
 */
#define list_for_each_entry_from(pos, head, type, member) \
  for (; &pos->member != (head); pos = list_next_entry(pos, type, member))

/**
 * list_for_each_entry_from_reverse - iterate backwards over list of given type
 *                                    from the current point
 * @pos:    the type * to use as a loop cursor.
 * @head:    the head for your list.
 * @member:    the name of the list_head within the struct.
 *
 * Iterate backwards over list of given type, continuing from current position.
 */
#define list_for_each_entry_from_reverse(pos, head, type, member) \
  for (; &pos->member != (head); pos = list_prev_entry(pos, type, member))

/**
 * list_for_each_entry_safe - iterate over list of given type safe against
 * removal of list entry
 * @pos:    the type * to use as a loop cursor.
 * @n:        another type * to use as temporary storage
 * @head:    the head for your list.
 * @member:    the name of the list_head within the struct.
 */
#define list_for_each_entry_safe(pos, n, head, type, member) \
  for (pos = list_first_entry(head, type, member),           \
      n = list_next_entry(pos, type, member);                \
       &pos->member != (head); pos = n, n = list_next_entry(n, type, member))

/**
 * list_for_each_entry_safe_continue - continue list iteration safe against
 * removal
 * @pos:    the type * to use as a loop cursor.
 * @n:        another type * to use as temporary storage
 * @head:    the head for your list.
 * @member:    the name of the list_head within the struct.
 *
 * Iterate over list of given type, continuing after current point,
 * safe against removal of list entry.
 */
#define list_for_each_entry_safe_continue(pos, n, head, type, member) \
  for (pos = list_next_entry(pos, type, member),                      \
      n = list_next_entry(pos, type, member);                         \
       &pos->member != (head); pos = n, n = list_next_entry(n, type, member))

/**
 * list_for_each_entry_safe_from - iterate over list from current point safe
 * against removal
 * @pos:    the type * to use as a loop cursor.
 * @n:        another type * to use as temporary storage
 * @head:    the head for your list.
 * @member:    the name of the list_head within the struct.
 *
 * Iterate over list of given type from current point, safe against
 * removal of list entry.
 */
#define list_for_each_entry_safe_from(pos, n, head, type, member)      \
  for (n = list_next_entry(pos, type, member); &pos->member != (head); \
       pos = n, n = list_next_entry(n, type, member))

/**
 * list_for_each_entry_safe_reverse - iterate backwards over list safe against
 * removal
 * @pos:    the type * to use as a loop cursor.
 * @n:        another type * to use as temporary storage
 * @head:    the head for your list.
 * @member:    the name of the list_head within the struct.
 *
 * Iterate backwards over list of given type, safe against removal
 * of list entry.
 */
#define list_for_each_entry_safe_reverse(pos, n, head, type, member) \
  for (pos = list_last_entry(head, type, member),                    \
      n = list_prev_entry(pos, type, member);                        \
       &pos->member != (head); pos = n, n = list_prev_entry(n, type, member))

/**
 * list_safe_reset_next - reset a stale list_for_each_entry_safe loop
 * @pos:    the loop cursor used in the list_for_each_entry_safe loop
 * @n:        temporary storage used in list_for_each_entry_safe
 * @member:    the name of the list_head within the struct.
 *
 * list_safe_reset_next is not safe to use in general if the list may be
 * modified concurrently (eg. the lock is dropped in the loop body). An
 * exception to this is if the cursor element (pos) is pinned in the list,
 * and list_safe_reset_next is called after re-taking the lock and before
 * completing the current iteration of the loop body.
 */
#define list_safe_reset_next(pos, n, type, member) \
  n = list_next_entry(pos, type, member)

/*
 * Double linked lists with a single pointer list head.
 * Mostly useful for hash tables where the two pointer list head is
 * too wasteful.
 * You lose the ability to access the tail in O(1).
 */
struct hlist_head {
  struct hlist_node *first;
};

struct hlist_node {
  struct hlist_node *next, **pprev;
};

#define HLIST_HEAD_INIT \
  { NULL }
#define HLIST_HEAD(name) struct hlist_head name = {NULL}
#define INIT_HLIST_HEAD(ptr) ((ptr)->first = NULL)
static void INIT_HLIST_NODE(struct hlist_node *h) {
  h->next = NULL;
  h->pprev = NULL;
}

/**
 * hlist_unhashed - Has node been removed from list and reinitialized?
 * @h: Node to be checked
 *
 * Not that not all removal functions will leave a node in unhashed
 * state.  For example, hlist_nulls_del_init_rcu() does leave the
 * node in unhashed state, but hlist_nulls_del() does not.
 */
static int hlist_unhashed(const struct hlist_node *h) { return !h->pprev; }

/**
 * hlist_unhashed_lockless - Version of hlist_unhashed for lockless use
 * @h: Node to be checked
 *
 * This variant of hlist_unhashed() must be used in lockless contexts
 * to avoid potential load-tearing.  The () is paired with the
 * various ) in hlist helpers that are defined below.
 */
static int hlist_unhashed_lockless(const struct hlist_node *h) {
  return !(h->pprev);
}

/**
 * hlist_empty - Is the specified hlist_head structure an empty hlist?
 * @h: Structure to check.
 */
static int hlist_empty(const struct hlist_head *h) { return !(h->first); }

static void __hlist_del(struct hlist_node *n) {
  struct hlist_node *next = n->next;
  struct hlist_node **pprev = n->pprev;

  *pprev = next;
  if (next) next->pprev = pprev;
}

/**
 * hlist_del - Delete the specified hlist_node from its list
 * @n: Node to delete.
 *
 * Note that this function leaves the node in hashed state.  Use
 * hlist_del_init() or similar instead to unhash @n.
 */
static void hlist_del(struct hlist_node *n) {
  __hlist_del(n);
  n->next = (struct hlist_node *)LIST_POISON1;
  n->pprev = (struct hlist_node **)LIST_POISON2;
}

/**
 * hlist_del_init - Delete the specified hlist_node from its list and initialize
 * @n: Node to delete.
 *
 * Note that this function leaves the node in unhashed state.
 */
static void hlist_del_init(struct hlist_node *n) {
  if (!hlist_unhashed(n)) {
    __hlist_del(n);
    INIT_HLIST_NODE(n);
  }
}

/**
 * hlist_add_head - add a new_node entry at the beginning of the hlist
 * @n: new_node entry to be added
 * @h: hlist head to add it after
 *
 * Insert a new_node entry after the specified head.
 * This is good for implementing stacks.
 */
static void hlist_add_head(struct hlist_node *n, struct hlist_head *h) {
  struct hlist_node *first = h->first;
  n->next = first;
  if (first) first->pprev = &n->next;
  h->first = n;
  n->pprev = &h->first;
}

/**
 * hlist_add_before - add a new_node entry before the one specified
 * @n: new_node entry to be added
 * @next: hlist node to add it before, which must be non-NULL
 */
static void hlist_add_before(struct hlist_node *n, struct hlist_node *next) {
  n->pprev = next->pprev;
  n->next = next;
  next->pprev = &n->next;
  *(n->pprev) = n;
}

/**
 * hlist_add_behing - add a new_node entry after the one specified
 * @n: new_node entry to be added
 * @prev: hlist node to add it after, which must be non-NULL
 */
static void hlist_add_behind(struct hlist_node *n, struct hlist_node *prev) {
  n->next = prev->next;
  prev->next = n;
  n->pprev = &prev->next;

  if (n->next) n->next->pprev = &n->next;
}

/**
 * hlist_add_fake - create a fake hlist consisting of a single headless node
 * @n: Node to make a fake list out of
 *
 * This makes @n appear to be its own predecessor on a headless hlist.
 * The point of this is to allow things like hlist_del() to work correctly
 * in cases where there is no list.
 */
static void hlist_add_fake(struct hlist_node *n) { n->pprev = &n->next; }

/**
 * hlist_fake: Is this node a fake hlist?
 * @h: Node to check for being a self-referential fake hlist.
 */
static int hlist_fake(struct hlist_node *h) { return h->pprev == &h->next; }

/**
 * hlist_is_singular_node - is node the only element of the specified hlist?
 * @n: Node to check for singularity.
 * @h: Header for potentially singular list.
 *
 * Check whether the node is the only node of the head without
 * accessing head, thus avoiding unnecessary cache misses.
 */
static int hlist_is_singular_node(struct hlist_node *n, struct hlist_head *h) {
  return !n->next && n->pprev == &h->first;
}

/**
 * hlist_move_list - Move an hlist
 * @old: hlist_head for old list.
 * @new_node: hlist_head for new_node list.
 *
 * Move a list from one list head to another. Fixup the pprev
 * reference of the first entry if it exists.
 */
static void hlist_move_list(struct hlist_head *old,
                            struct hlist_head *new_node) {
  new_node->first = old->first;
  if (new_node->first) new_node->first->pprev = &new_node->first;
  old->first = NULL;
}

#define hlist_entry(ptr, type, member) container_of(ptr, type, member)

#define hlist_for_each(pos, head) \
  for (pos = (head)->first; pos; pos = pos->next)

#define hlist_for_each_safe(pos, n, head)    \
  for (pos = (head)->first; pos && ({        \
                              n = pos->next; \
                              1;             \
                            });              \
       pos = n)

// #define hlist_entry_safe(ptr, type, member)              \
//   ({                                                     \
//     struct hlist_node *____ptr = (ptr);                  \
//     ____ptr ? hlist_entry(____ptr, type, member) : NULL; \
//   })
#define hlist_entry_safe(ptr, type, member)              \
  (ptr ? hlist_entry(ptr, type, member) : NULL)

/**
 * hlist_for_each_entry    - iterate over list of given type
 * @pos:    the type * to use as a loop cursor.
 * @head:    the head for your list.
 * @member:    the name of the hlist_node within the struct.
 */
#define hlist_for_each_entry(pos, head, type, member)            \
  for (pos = hlist_entry_safe((head)->first, type, member); pos; \
       pos = hlist_entry_safe((pos)->member.next, type, member))

/**
 * hlist_for_each_entry_continue - iterate over a hlist continuing after current
 * point
 * @pos:    the type * to use as a loop cursor.
 * @member:    the name of the hlist_node within the struct.
 */
#define hlist_for_each_entry_continue(pos, type, member)              \
  for (pos = hlist_entry_safe((pos)->member.next, type, member); pos; \
       pos = hlist_entry_safe((pos)->member.next, type, member))

/**
 * hlist_for_each_entry_from - iterate over a hlist continuing from current
 * point
 * @pos:    the type * to use as a loop cursor.
 * @member:    the name of the hlist_node within the struct.
 */
#define hlist_for_each_entry_from(pos, type, member) \
  for (; pos; pos = hlist_entry_safe((pos)->member.next, type, member))

/**
 * hlist_for_each_entry_safe - iterate over list of given type safe against
 * removal of list entry
 * @pos:    the type * to use as a loop cursor.
 * @n:        another &struct hlist_node to use as temporary storage
 * @head:    the head for your list.
 * @member:    the name of the hlist_node within the struct.
 */
#define hlist_for_each_entry_safe(pos, n, head, type, member) \
  for (pos = hlist_entry_safe((head)->first, type, member);   \
       pos && ({                                              \
         n = pos->member.next;                                \
         1;                                                   \
       });                                                    \
       pos = hlist_entry_safe(n, type, member))

#endif  // LIST_H_20200320
