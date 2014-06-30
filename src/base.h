#ifndef BASE_H
#define BASE_H

/*
 * Name: Leonard Hecker
 * Studiengruppe: 13/041/61
 * MatrNr: XXXXX
 */


#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


typedef struct cnct_s tCnct;
typedef struct list_s tList;

typedef enum base_col_s     base_col_t;

typedef struct buf_s        buf_t;
typedef struct base_s       base_t;
typedef struct base_iter_s  base_iter_t;
typedef struct base_entry_s base_entry_t;


enum base_col_s {
	BASE_COL_NUMBER = 0,
	BASE_COL_NAME,
	BASE_COL_FORENAME,
	BASE_COL_COUNT,
};


struct buf_s {
	char *base;
	size_t len;
};

struct base_s {
	buf_t filename;

	/*
	 * Private variables.
	 * DO NOT access them or the internal state will be undefined.
	 */
	tList *_list;     // the data container
	size_t _listSize; // cached size of the tList
};

struct base_iter_s {
	tCnct *cnct;
	size_t index;
};

struct base_entry_s {
	buf_t number;
	buf_t name;
	buf_t fname;
	tCnct *cnct; // circular reference to the container
};

/**
 * Opens a database with the name specified as the first parameter
 * and creates a new base_t context, containing a deque with the entries.
 */
base_t *base_new(char *filename);

/**
 * Saves the data to the disk, overwriting the file passed in base_new().
 */
bool base_save(base_t *self);

/**
 * Frees all allocated memory associated with the base_t instance including itself.
 */
void base_delete(base_t *self);


/**
 * Returns the amount of entries in the list.
 */
size_t base_get_count(base_t *self);

/**
 * Returns an iterator pointing at the head of the list.
 */
base_iter_t base_get_begin(base_t *self);

/**
 * Returns an iterator pointing at the tail of the list.
 */
base_iter_t base_get_end(base_t *self);

/**
 * Returns an iterator pointing at the nth. element of the list.
 */
base_iter_t base_get_nth(base_t *self, size_t idx);

/**
 * Increments an iterator.
 */
void base_iter_next(base_iter_t *self);

/**
 * Decrements an iterator.
 */
void base_iter_prev(base_iter_t *self);

/**
 * Returns the value of the element an iterator is pointing at.
 */
base_entry_t *base_iter_value(base_iter_t *self);

/**
 * Returns the position of an iterator in the list.
 */
size_t base_iter_index(base_iter_t *self);


/**
 * Adds an entry to the head of the list.
 */
bool base_prepend(base_t *self, const char *number, const char *name, const char *fname);

/**
 * Adds an empty entry to the head of the list.
 */
bool base_prepend_placeholder(base_t *self);

/**
 * Adds the entry in the list with the given phone number.
 */
bool base_entry_remove(base_t *self, base_entry_t *entry);

/**
 * Changes an entry of the list.
 */
bool base_entry_change(base_t *self, base_entry_t *entry, const char *value, base_col_t col);

#endif
