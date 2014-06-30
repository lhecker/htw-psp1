/*
 * Name: Leonard Hecker
 * Studiengruppe: 13/041/61
 * MatrNr: XXXXX
 */

#include "list.h"

#include <assert.h>
#include <stdlib.h>


tList *createList(void) {
	return calloc(1, sizeof(tList));
}

int deleteList(tList *pList) {
	assert(pList);

	tCnct *current = pList->head;

	while (current) {
		tCnct *next = current->next;
		free(current);
		current = next;
	}

	free(pList);

	return OK;
}


void resetHeadTailCurrent(tList *pList, tCnct *current) {
	if (current) {
		if (!current->prev) {
			pList->head = current;
		}

		if (!current->next) {
			pList->tail = current;
		}

		pList->current = current;
	} else {
		pList->head = NULL;
		pList->tail = NULL;
		pList->current = NULL;
	}
}


int insertBehind(tList *pList, void *pItemIns) {
	assert(pList);

	tCnct *container = malloc(sizeof(tCnct));
	if (!container) {
		return FAIL;
	}

	tCnct *current = pList->current;
	if (current) {
		tCnct *next = current->next;

		if (next) {
			next->prev = container;
		}

		current->next = container;
		container->prev = current;
		container->next = next;
	} else {
		container->prev = NULL;
		container->next = NULL;
	}

	container->data = pItemIns;

	resetHeadTailCurrent(pList, container);

	return OK;
}

int insertBefore(tList *pList, void *pItemIns) {
	assert(pList);

	tCnct *container = malloc(sizeof(tCnct));
	if (!container) {
		return FAIL;
	}

	tCnct *current = pList->current;
	if (current) {
		tCnct *prev = current->prev;

		if (prev) {
			prev->next = container;
		}

		current->prev = container;
		container->prev = prev;
		container->next = current;
	} else {
		container->prev = NULL;
		container->next = NULL;
	}

	container->data = pItemIns;

	resetHeadTailCurrent(pList, container);

	return OK;
}

int insertHead(tList *pList, void *pItemIns) {
	assert(pList);

	pList->current = pList->head;
	return insertBefore(pList, pItemIns);
}

int insertTail(tList *pList, void *pItemIns) {
	assert(pList);

	pList->current = pList->tail;
	return insertBehind(pList, pItemIns);
}

int addItemToList(tList *pList, void *pItem, list_cmp_t fcmp) {
	assert(pList);
	assert(fcmp);

	tCnct *current = getFirst(pList);

	while (current && fcmp(pItem, pList->current) < 0) {
		current = getNext(pList);
	}

	return insertBehind(pList, pItem);
}


void removeItem(tList *pList) {
	assert(pList);

	tCnct *current = pList->current;

	if (current) {
		tCnct *prev = current->prev;
		tCnct *next = current->next;

		if (prev) {
			prev->next = next;
		}

		if (next) {
			next->prev = prev;
		}

		free(current);

		resetHeadTailCurrent(pList, prev ? prev : next);
	}
}


void *getSelected(tList *pList) {
	assert(pList);

	return pList->current ? pList->current->data : NULL;
}

void *getFirst(tList *pList) {
	assert(pList);

	pList->current = pList->head;
	return getSelected(pList);
}

void *getLast(tList *pList) {
	assert(pList);

	pList->current = pList->tail;
	return getSelected(pList);
}

void *getNext(tList *pList) {
	assert(pList);

	if (pList->current) {
		pList->current = pList->current->next;
	}

	return getSelected(pList);
}

void *getPrev(tList *pList) {
	assert(pList);

	if (pList->current) {
		pList->current = pList->current->prev;
	}

	return getSelected(pList);
}

void *getIndexed(tList *pList, int idx) {
	assert(pList);

	tCnct *container = pList->head;

	for (int i = 0; container && i < idx; container = container->next, i++);

	if (container) {
		pList->current = container;
		return getSelected(pList);
	} else {
		return NULL;
	}
}
