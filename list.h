  /*  list.h
   *  David Töyrä
   *  Obligatorisk uppgift 1 Systemnära programmering HT17
   *  2017-09-07
   *  Header for list.c
   */
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
typedef void *data;
typedef void memFreeFunc(data);

typedef struct Node
{
	data value;
	struct Node *nextNode;
	struct Node *prevNode;
} Node;

typedef struct list
{
	Node *head;
	memFreeFunc *freeFunc;
}list;

typedef Node *listPosition;

/*
 * Makes an empty list, pointers for previous and next node
 * points to NULL.
 */
list *list_makeList(void);

/*
 * Set up memory handler.
 */
void list_setMemHandler(list *l, memFreeFunc *f);

/*
 *Returns the first node of the list.
 *If list is empty returns head.
 */
listPosition list_first(list *l);

/*
 * Returns the next node.
 */
listPosition list_next(listPosition p);

/*
 * Returns the end node, it holds no value.
 */
listPosition list_end(list *l);
/*
 * Returns the previous node.
 */
listPosition list_prev(listPosition p);
/*
 * Checks if the list empty.
 */
bool list_isEmpty(list *l);

/*
 * Inserts a node into the list structure.
 * Node will be inserted before the position p, should the list be empty 
 * the node will be inserted after the head.
 */
listPosition list_insert(list *l, data d, listPosition p);

/*
 * Get the data of a given list position.
 */
data list_inspect(listPosition p);

/*
 * Remove a node from the list, allocated memory will be free'd.
 */
listPosition list_remove(list *l, listPosition p);

/*
 * Remove the entire list, allocated memory will be free'd.
 */
void list_free(list *l);
