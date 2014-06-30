/*
 * Name: Leonard Hecker
 * Studiengruppe: 13/041/61
 * MatrNr: XXXXX
 */

#include "base.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"


static const char *kEmptyString = "";


bool readLengthPrefixedString(FILE *fp, buf_t *buf) {
	assert(fp);
	assert(buf);

	size_t len = 0;
	size_t read;

	read = fread(&len, sizeof(uint8_t), 1, fp);

	if (read == 1) {
		char *base = malloc(len + 1);

		if (base) {
			read = fread(base, sizeof(char), len, fp);

			if (read == len) {
				// support for 0-terminated strings
				base[len] = 0;

				buf->base = base;
				buf->len = len;
				return true;
			}
		}

		free(base);
	}

	buf->base = NULL;
	buf->len = 0;

	return false;
}

bool writeLengthPrefixedString(FILE *fp, buf_t buf) {
	assert(fp);
	assert(buf.base);
	assert(buf.len < UINT8_MAX);

	size_t written;

	written = fwrite(&buf.len, sizeof(uint8_t), 1, fp);

	if (written != 1) {
		return false;
	}

	written = fwrite(buf.base, sizeof(char), buf.len & UINT8_MAX, fp);
	return written == buf.len;
}


base_t *base_new(char *filename) {
	base_t *self = malloc(sizeof(base_t));

	if (!self) {
		return NULL;
	}

	{
		buf_t *buf = &self->filename;

		buf->len = strlen(filename);
		buf->base = malloc(buf->len + 1);

		if (!buf->base) {
			base_delete(self);
			return NULL;
		}

		// ensure compatibility with legacy functions using 0-terminated strings
		memcpy(buf->base, filename, buf->len);
		buf->base[buf->len] = 0;
	}

	self->_list = createList();

	if (!self->_list) {
		base_delete(self);
		return NULL;
	}

	size_t listSize = 0;
	FILE *fp = fopen(filename, "rb");

	if (fp) {
		while (true) {
			buf_t number;
			buf_t name;
			buf_t fname;

			bool ok1 = readLengthPrefixedString(fp, &number);
			bool ok2 = readLengthPrefixedString(fp, &name);
			bool ok3 = readLengthPrefixedString(fp, &fname);

			if (ok1 && ok2 && ok3) {
				base_entry_t *entry = malloc(sizeof(base_entry_t));

				if (entry) {
					entry->number = number;
					entry->name = name;
					entry->fname = fname;

					insertTail(self->_list, entry);
					entry->cnct = self->_list->current;

					listSize++;
				}
			} else {
				break;
			}
		}

		fclose(fp);
	}

	self->_listSize = listSize;

	return self;
}

bool base_save(base_t *self) {
	assert(self);

	FILE *fp = fopen(self->filename.base, "wb");

	if (!fp) {
		return false;
	}

	base_entry_t *entry = getFirst(self->_list);
	bool ret = true;

	while (entry) {
		if (entry->number.len && entry->name.len && entry->fname.len) {
			bool ok1 = writeLengthPrefixedString(fp, entry->number);
			bool ok2 = writeLengthPrefixedString(fp, entry->name);
			bool ok3 = writeLengthPrefixedString(fp, entry->fname);

			if (!ok1 || !ok2 || !ok3) {
				ret = false;
				break;
			}
		}

		entry = getNext(self->_list);
	}

	fclose(fp);

	return ret;
}

void base_delete(base_t *self) {
	if (self) {
		if (self->_list) {
			base_entry_t *entry = getFirst(self->_list);

			while (entry) {
				char *numberBase = entry->number.base;
				char *nameBase = entry->name.base;
				char *fnameBase = entry->fname.base;

				if (numberBase != kEmptyString) {
					free(numberBase);
				}

				if (nameBase != kEmptyString) {
					free(nameBase);
				}

				if (fnameBase != kEmptyString) {
					free(fnameBase);
				}

				free(entry);

				entry = getNext(self->_list);
			}

			deleteList(self->_list);
		}

		free(self->filename.base);
		free(self);
	}
}

size_t base_get_count(base_t *self) {
	assert(self);

	return self->_listSize;
}

base_iter_t base_get_begin(base_t *self) {
	assert(self);

	base_iter_t iter = { .cnct = self->_list->head, .index = 0 };
	return iter;
}

base_iter_t base_get_end(base_t *self) {
	assert(self);

	base_iter_t iter = { .cnct = self->_list->tail, .index = base_get_count(self) - 1 };
	return iter;
}

base_iter_t base_get_nth(base_t *self, size_t idx) {
	assert(self);

	tCnct *cnct = self->_list->head;
	size_t i = 0;

	for (; cnct && i < idx; i++) {
		cnct = cnct->next;
	}

	if (!cnct) {
		i = 0;
	}

	base_iter_t iter = { .cnct = cnct, .index = i };
	return iter;
}

void base_iter_next(base_iter_t *self) {
	assert(self);

	tCnct *cnct = self->cnct;

	if (cnct) {
		cnct = cnct->next;
		self->cnct = cnct;

		if (cnct) {
			self->index++;
		}
	}
}

void base_iter_prev(base_iter_t *self) {
	assert(self);

	tCnct *cnct = self->cnct;

	if (cnct) {
		cnct = cnct->prev;
		self->cnct = cnct;

		if (cnct) {
			assert(self->index > 0);
			self->index--;
		}
	}
}

base_entry_t *base_iter_value(base_iter_t *self) {
	assert(self);

	tCnct *cnct = self->cnct;
	return cnct ? cnct->data : NULL;
}

size_t base_iter_index(base_iter_t *self) {
	assert(self);

	return self->index;
}

bool base_prepend(base_t *self, const char *number, const char *name, const char *fname) {
	assert(self);
	assert(number);
	assert(name);
	assert(fname);

	size_t numberLen = strlen(number);
	size_t nameLen   = strlen(name);
	size_t fnameLen  = strlen(fname);

	if (numberLen && nameLen && fnameLen) {
		base_entry_t *entry = malloc(sizeof(base_entry_t));
		char *numberBase = malloc(numberLen + 1);
		char *nameBase   = malloc(fnameLen  + 1);
		char *fnameBase  = malloc(fnameLen  + 1);

		if (entry && numberBase && nameBase && fnameBase) {
			memcpy(numberBase, number, numberLen + 1);
			memcpy(nameBase,   name,   nameLen   + 1);
			memcpy(fnameBase,  fname,  fnameLen  + 1);

			entry->number = (buf_t) { .base = numberBase, .len = numberLen };
			entry->name   = (buf_t) { .base = nameBase,   .len = nameLen   };
			entry->fname  = (buf_t) { .base = fnameBase,  .len = fnameLen  };

			insertHead(self->_list, entry);
			entry->cnct = self->_list->current;

			self->_listSize++;

			return true;
		}

		free(entry);
		free(numberBase);
		free(nameBase);
		free(fnameBase);
	}

	return false;
}

bool base_prepend_placeholder(base_t *self) {
	assert(self);

	base_entry_t *entry = malloc(sizeof(base_entry_t));

	if (entry) {
		entry->number = entry->name = entry->fname = (buf_t) { .base = (char *)kEmptyString, .len = 0 };

		insertHead(self->_list, entry);
		entry->cnct = self->_list->current;

		self->_listSize++;

		return true;
	}

	return false;
}

bool base_entry_remove(base_t *self, base_entry_t *entry) {
	assert(self);
	assert(entry);

	if (self->_listSize > 0 && entry->number.len && entry->name.len && entry->fname.len) {
		free(entry->number.base);
		free(entry->name.base);
		free(entry->fname.base);
		free(entry);

		self->_list->current = entry->cnct;
		removeItem(self->_list);

		self->_listSize--;

		return true;
	}

	return false;
}

bool base_entry_change(base_t *self, base_entry_t *entry, const char *value, base_col_t col) {
	assert(self);
	assert(entry);
	assert(value);
	assert(col < BASE_COL_COUNT);

	(void)(self); // unused

	size_t len = strlen(value);

	if (len > 0 && len <= UINT8_MAX) {
		char *base = malloc(len + 1);

		if (base) {
			memcpy(base, value, len + 1);

			buf_t *buf;

			switch (col) {
				case BASE_COL_NUMBER:
					buf = &entry->number;
					break;
				case BASE_COL_NAME:
					buf = &entry->name;
					break;
				case BASE_COL_FORENAME:
					buf = &entry->fname;
					break;
				default:
					assert(false);
					return false;
			}

			if (buf->base != kEmptyString) {
				free(buf->base);
			}

			buf->base = base;
			buf->len = len;

			return true;
		}
	}

	return false;
}
