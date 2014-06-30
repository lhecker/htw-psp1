#ifndef LIST_H
#define LIST_H

/*
 * Name: Leonard Hecker
 * Studiengruppe: 13/041/61
 * MatrNr: XXXXX
 */

/*
 * I highly dislike the idea of saving a "current" pointer in a double linked list,
 * simply because since there should be the ability to create multiple iterators.
 * Furthermore the API employs no naming convention and uses ordinary names,
 * which possibly leads to function name collisions and confusion.
 *
 * The API spec should be reconsidered and conform to the more common ones in the future.
 */


#define OK 1
#define FAIL 0


typedef struct cnct_s tCnct;
typedef struct list_s tList;

typedef int(*list_cmp_t)(void *pItList, void *pItNew);


struct cnct_s {
	tCnct *prev;
	tCnct *next;
	void *data;
};

struct list_s {
	tCnct *head;
	tCnct *tail;
	tCnct *current;
};


/**
 * Creates an new, empty list.
 */
tList *createList(void);

/**
 * Frees pList and all entries. DOES NOT free() the data of list entries itself.
 */
int deleteList(tList *pList);

/**
 * Searches for pItemIns and if found inserts after the current element.
 */
int insertBehind(tList *pList, void *pItemIns);

/**
 * Searches for pItemIns and if found inserts before the current element.
 */
int insertBefore(tList *pList, void *pItemIns);

/**
 * Prepends pItemIns to the current head element.
 */
int insertHead(tList *pList, void *pItemIns);

/**
 * Appends pItemIns to the current tail element.
 */
int insertTail(tList *pList, void *pItemIns);

/**
 * Inserts pItem into the list in a sorted manner using the comperator callback fcmp.
 */
int addItemToList(tList *pList, void *pItem, list_cmp_t fcmp);


/**
 * Removes the current element.
 */
void removeItem(tList *pList);


/**
 * Returns the content of the current element.
 */
void *getSelected(tList *pList);

/**
 * Sets the current pointer to the head element and returns it's content.
 */
void *getFirst(tList *pList);

/**
 * Sets the current pointer to the tail element and returns it's content.
 */
void *getLast(tList *pList);

/**
 * Sets the current pointer to the next element and returns it's content.
 */
void *getNext(tList *pList);

/**
 * Sets the current pointer to the previous element and returns it's content.
 */
void *getPrev(tList *pList);

/**
 * Sets the current pointer to the element at position idx and returns it's content.
 */
void *getIndexed(tList *pList, int idx);


#endif
