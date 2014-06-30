#ifndef GBASE_LIST_PRIVATE_H
#define GBASE_LIST_PRIVATE_H

/*
 * Name: Leonard Hecker
 * Studiengruppe: 13/041/61
 * MatrNr: XXXXX
 */

/*
 * This module is based on http://en.wikibooks.org/wiki/GTK+_By_Example/Tree_View/Custom_Models
 */


#include <base.h>
#include <gtk/gtk.h>


#define GBASE_TYPE_LIST            (gbase_list_get_type())
#define GBASE_LIST(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), GBASE_TYPE_LIST, GBaseList))
#define GBASE_LIST_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), GBASE_TYPE_LIST, GBaseListClass))
#define GBASE_IS_LIST(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), GBASE_TYPE_LIST))
#define GBASE_IS_LIST_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), GBASE_TYPE_LIST))
#define GBASE_LIST_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), GBASE_TYPE_LIST, GBaseListClass))


// the data columns that we export via the tree model interface
enum {
	GBASE_LIST_COL_NUMBER   = BASE_COL_NUMBER,
	GBASE_LIST_COL_NAME     = BASE_COL_NAME,
	GBASE_LIST_COL_FORENAME = BASE_COL_FORENAME,
	GBASE_LIST_COL_DATA,
	GBASE_LIST_N_COLUMNS,
};


typedef struct _GBaseListClass GBaseListClass;
typedef struct _GBaseList      GBaseList;
typedef struct _GBaseRecord    GBaseRecord;


struct _GBaseList {
	GObject parent_instance;

	base_t *base_ctx;

	/*
	 * Random integer to check whether an iter belongs to our model
	 */
	gint stamp;

	/*
	 * These two fields are not absolutely necessary, but they
	 * speed things up a bit in our get_value implementation
	 */
	gint n_columns;
	GType column_types[GBASE_LIST_N_COLUMNS];
};

struct _GBaseListClass {
	GObjectClass parent_class;
};

/*
 * Required by GTK's class system.
 */
GType gbase_list_get_type();

/*
 * Instantiates a new GBaseList wrapper based on a base_t.
 */
GBaseList *gbase_list_new(base_t *ctx);

/*
 * Adds a row at the beginning of the list.
 */
void gbase_list_prepend_record(GBaseList *self, const gchar *number, const gchar *name, const gchar *fname);

/*
 * Changes a single field at the specified row.
 * Returns true if it was the placeholder row, whose fields are now all full
 * due to which a new placeholder row had to be added.
 */
bool gbase_list_change_record(GBaseList *self, const gchar *value, unsigned int row, unsigned int col);

/*
 * Deletes the specified row.
 */
void gbase_list_delete_record(GBaseList *self, GtkTreeIter *iter);

#endif
