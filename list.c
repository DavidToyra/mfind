  /*  list.c
   *  David Töyrä
   *  Obligatorisk uppgift 1 Systemnära programmering HT17
   *  2017-09-07
   *  
   *  Double linked list.
   */

#include "list.h"

/*
 * Makes an empty list, set pointers to NULL.
 */
list *list_makeList(void)
{
	list *newList = malloc(sizeof(list));
	newList->head = malloc(sizeof(Node));
	newList->head->nextNode = newList->head;
	newList->head->prevNode = newList->head;
	newList->freeFunc = NULL;
	return newList;
}

/*
 * Set up memory handler.
 */
void list_setMemHandler(list *l, memFreeFunc *f) 
{
   l->freeFunc = f;
}
/*
 *Returns the first node of the list.
 *If list is empty returns head.
 */
listPosition list_first(list *l)
{
	if(list_isEmpty(l))
	{
		return l->head;
	}
	
	return l->head->nextNode;
}

/*
 * Returns the end node, it holds no value.
 */
listPosition list_end(list *l)
{
	return l->head->prevNode;
}

/*
 * Returns the next node, if pointer points to NULL the program will exit.
 */
listPosition list_next(listPosition p)
{
	if(p->nextNode==NULL)
	{
		perror("Next node is NULL");
		EXIT_FAILURE;
	}
	
	return p->nextNode;
}

listPosition list_prev(listPosition p)
{
	if(p->prevNode==NULL)
	{
		perror("Previous node is NULL");
		EXIT_FAILURE;
	}
	return p->prevNode;
}

/*
 * Checks if the list empty, returns the boolean value of
 * wheter the head points to NULL.
 */
 bool list_isEmpty(list *l)
{
	return (l->head->nextNode==l->head);
}

/*
 * Inserts a node into the list structure.
 * Node will be inserted before the position p, should the list be empty 
 * the node will be inserted after the head.
 */
listPosition list_insert(list *l, data d, listPosition p)
{
	listPosition newNode = malloc(sizeof(struct Node));
	newNode->value = d;
	
	newNode->prevNode = p->prevNode;
	newNode->nextNode = p;
	//Notera att man mÃ¥ste fÃ¶rst Ã¤ndra pekaren pÃ¥ den noden lÃ¤ngst Ã¥t vÃ¤nster i listan!
	//Annars kan man inte hitta den med "prevNode" nÃ¥ mer!
	p->prevNode->nextNode = newNode;
	p->prevNode = newNode;
	return newNode;
}

/*
 * Returns the value of a node.
 */
data list_inspect(listPosition p)
{
	return p->value;
}

/*
 * Remove a node from the list, allocated memory will be free'd.
 */
listPosition list_remove(list *l, listPosition p)
{

	listPosition returnPos = p->nextNode;
	p->prevNode->nextNode = p->nextNode;
	p->nextNode->prevNode = p->prevNode;
	
	if(l->freeFunc!=NULL)
        l->freeFunc(p->value);

	free(p);
	return returnPos;
}

/*
 * Remove the entire list, allocated memory will be free'd.
 */
void list_free(list *l)
{
	listPosition p;

    while(!list_isEmpty(l))
	{
        p = list_first(l);
		list_remove(l, p);
    }
    
	free(l->head);
    free(l);
    return;
}
