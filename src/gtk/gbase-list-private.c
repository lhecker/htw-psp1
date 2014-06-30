/*
 * Name: Leonard Hecker
 * Studiengruppe: 13/041/61
 * MatrNr: XXXXX
 */

#include "gbase-list-private.h"

#include <string.h>


#define USER_DATA_2_BASE_ITER(ptr) ((base_iter_t *)&(ptr))


static void gbase_list_init(GBaseList *pkg_tree);
static void gbase_list_class_init(GBaseListClass *klass);
static void gbase_list_tree_model_init(GtkTreeModelIface *iface);
static void gbase_list_finalize(GObject *object);

static GtkTreeModelFlags gbase_list_get_flags(GtkTreeModel *tree_model);
static gint gbase_list_get_n_columns(GtkTreeModel *tree_model);
static GType gbase_list_get_column_type(GtkTreeModel *tree_model, gint index);
static gboolean gbase_list_get_iter(GtkTreeModel *tree_model, GtkTreeIter *iter, GtkTreePath *path);
static GtkTreePath *gbase_list_get_path(GtkTreeModel *tree_model, GtkTreeIter *iter);
static void gbase_list_get_value(GtkTreeModel *tree_model, GtkTreeIter *iter, gint column, GValue *value);
static gboolean gbase_list_iter_next(GtkTreeModel *tree_model, GtkTreeIter *iter);
static gboolean gbase_list_iter_children(GtkTreeModel *tree_model, GtkTreeIter *iter, GtkTreeIter *parent);
static gboolean gbase_list_iter_has_child(GtkTreeModel *tree_model, GtkTreeIter *iter);
static gint gbase_list_iter_n_children(GtkTreeModel *tree_model, GtkTreeIter *iter);
static gboolean gbase_list_iter_nth_child(GtkTreeModel *tree_model, GtkTreeIter *iter, GtkTreeIter *parent, gint n);
static gboolean gbase_list_iter_parent(GtkTreeModel *tree_model, GtkTreeIter *iter, GtkTreeIter *child);

static GObjectClass *parent_class = NULL;  /* GObject stuff - nothing to worry about */


/*
 * more boilerplate GObject/GType stuff.
 * Init callback for the type system,
 * called once when our new class is created.
 */
static void gbase_list_class_init(GBaseListClass *klass) {
	GObjectClass *object_class = (GObjectClass *)klass;
	object_class->finalize = gbase_list_finalize;

	parent_class = (GObjectClass *)g_type_class_peek_parent(klass);
}

/*
 * init callback for the interface registration
 * in gbase_list_get_type. Here we override
 * the GtkTreeModel interface functions that
 * we implement.
 */
static void gbase_list_tree_model_init(GtkTreeModelIface *iface) {
	iface->get_flags       = gbase_list_get_flags;
	iface->get_n_columns   = gbase_list_get_n_columns;
	iface->get_column_type = gbase_list_get_column_type;
	iface->get_iter        = gbase_list_get_iter;
	iface->get_path        = gbase_list_get_path;
	iface->get_value       = gbase_list_get_value;
	iface->iter_next       = gbase_list_iter_next;
	iface->iter_children   = gbase_list_iter_children;
	iface->iter_has_child  = gbase_list_iter_has_child;
	iface->iter_n_children = gbase_list_iter_n_children;
	iface->iter_nth_child  = gbase_list_iter_nth_child;
	iface->iter_parent     = gbase_list_iter_parent;
}


/*
 * this is called everytime a new custom list object
 * instance is created (we do that in gbase_list_new).
 * Initialise the list structure's fields here.
 */
static void gbase_list_init(GBaseList *gbase_list) {
	gbase_list->base_ctx = NULL;
	gbase_list->stamp = g_random_int();  /* Random int to check whether an iter belongs to our model */

	gbase_list->n_columns = GBASE_LIST_N_COLUMNS;
	gbase_list->column_types[GBASE_LIST_COL_NUMBER]   = G_TYPE_STRING;
	gbase_list->column_types[GBASE_LIST_COL_NAME]     = G_TYPE_STRING;
	gbase_list->column_types[GBASE_LIST_COL_FORENAME] = G_TYPE_STRING;
	gbase_list->column_types[GBASE_LIST_COL_DATA]     = G_TYPE_POINTER;
}


/*
 * this is called just before a custom list is
 * destroyed. Free dynamically allocated memory here.
 */
static void gbase_list_finalize(GObject *object) {
	G_OBJECT_CLASS(parent_class)->finalize(object);
}


/*
 * tells the rest of the world whether our tree model
 * has any special characteristics. In our case,
 * we have a list model (instead of a tree), and each
 * tree iter is valid as long as the row in question
 * exists, as it only contains a pointer to our struct.
 */

static GtkTreeModelFlags gbase_list_get_flags(GtkTreeModel *tree_model) {
	g_return_val_if_fail(GBASE_IS_LIST(tree_model), (GtkTreeModelFlags)0);

	return (GTK_TREE_MODEL_LIST_ONLY | GTK_TREE_MODEL_ITERS_PERSIST);
}


/*
 * tells the rest of the world how many data
 * columns we export via the tree model interface
 */
static gint gbase_list_get_n_columns(GtkTreeModel *tree_model) {
	g_return_val_if_fail(GBASE_IS_LIST(tree_model), 0);

	return GBASE_LIST(tree_model)->n_columns;
}


/*
 * tells the rest of the world which type of
 * data an exported model column contains
 */
static GType gbase_list_get_column_type(GtkTreeModel *tree_model, gint index) {
	g_return_val_if_fail(GBASE_IS_LIST(tree_model), G_TYPE_INVALID);
	g_return_val_if_fail(index < GBASE_LIST(tree_model)->n_columns && index >= 0, G_TYPE_INVALID);

	return GBASE_LIST(tree_model)->column_types[index];
}


/*
 * converts a tree path (physical position) into a
 * tree iter structure (the content of the iter
 * fields will only be used internally by our model).
 * We simply store a pointer to our GBaseRecord
 * structure that represents that row in the tree iter.
 */
static gboolean gbase_list_get_iter(GtkTreeModel *tree_model, GtkTreeIter *iter, GtkTreePath *path) {
	g_assert(GBASE_IS_LIST(tree_model));
	g_assert(path != NULL);

	GBaseList *self = GBASE_LIST(tree_model);

	/*
	 * Fetch the path in the tree. Out tree is flat like a list and has therefore a depth of 1.
	 */
	gint *indices = gtk_tree_path_get_indices(path);
	gint depth = gtk_tree_path_get_depth(path);
	g_assert(indices != NULL);
	g_assert(depth == 1);

	gint idx = indices[0];
	base_iter_t baseIter = base_get_nth(self->base_ctx, idx);

	if (!baseIter.cnct) {
		return FALSE;
	}

	iter->stamp = self->stamp;

	/*
	 * HACK: GTK's GtkTreeIter has 3 * sizeof(void*) of user data space,
	 * which we directly use as cheap storage
	 */
	*USER_DATA_2_BASE_ITER(iter->user_data) = baseIter;

	return TRUE;
}


/*
 * converts a tree iter into a tree path (ie. the
 * physical position of that row in the list).
 */
static GtkTreePath *gbase_list_get_path(GtkTreeModel *tree_model, GtkTreeIter *iter) {
	g_return_val_if_fail(GBASE_IS_LIST(tree_model), NULL);
	g_return_val_if_fail(iter != NULL, NULL);

	GtkTreePath *path = gtk_tree_path_new();
	gtk_tree_path_append_index(path, base_iter_index(USER_DATA_2_BASE_ITER(iter->user_data)));

	return path;
}


/*
 * Returns a row's exported data columns
 * (_get_value is what gtk_tree_model_get uses)
 */
static void gbase_list_get_value(GtkTreeModel *tree_model, GtkTreeIter *iter, gint column, GValue *value) {
	g_return_if_fail(GBASE_IS_LIST(tree_model));
	g_return_if_fail(iter != NULL);
	g_return_if_fail(column < GBASE_LIST(tree_model)->n_columns);

	g_value_init(value, GBASE_LIST(tree_model)->column_types[column]);

	base_entry_t *entry = base_iter_value(USER_DATA_2_BASE_ITER(iter->user_data));
	g_return_if_fail(entry != NULL);

	switch (column) {
		case GBASE_LIST_COL_NUMBER:
			g_value_set_string(value, entry->number.base);
			break;

		case GBASE_LIST_COL_NAME:
			g_value_set_string(value, entry->name.base);
			break;

		case GBASE_LIST_COL_FORENAME:
			g_value_set_string(value, entry->fname.base);
			break;

		case GBASE_LIST_COL_DATA:
			g_value_set_pointer(value, entry);
			break;

		default:
			g_assert(FALSE);
			break;
	}
}


/*
 * Takes an iter structure and sets it to point
 * to the next row.
 */
static gboolean gbase_list_iter_next(GtkTreeModel *tree_model, GtkTreeIter *iter) {
	g_return_val_if_fail(GBASE_IS_LIST(tree_model), FALSE);
	g_return_val_if_fail(iter != NULL, FALSE);

	GBaseList *self = GBASE_LIST(tree_model);
	base_iter_next(USER_DATA_2_BASE_ITER(iter->user_data));

	if (!base_iter_value(USER_DATA_2_BASE_ITER(iter->user_data))) {
		return FALSE;
	}

	iter->stamp = self->stamp;

	return TRUE;
}


/*
 * Returns TRUE or FALSE depending on whether
 * the row specified by 'parent' has any children.
 * If it has children, then 'iter' is set to
 * point to the first child. Special case: if
 * 'parent' is NULL, then the first top-level
 * row should be returned if it exists.
 */
static gboolean gbase_list_iter_children(GtkTreeModel *tree_model, GtkTreeIter *iter, GtkTreeIter *parent) {
	g_return_val_if_fail(GBASE_IS_LIST(tree_model), FALSE);
	g_return_val_if_fail(iter != NULL, FALSE);

	// this is a list - nodes have no children
	if (parent) {
		return FALSE;
	}

	GBaseList *self = GBASE_LIST(tree_model);
	base_iter_t baseIter = base_get_begin(self->base_ctx);

	if (!baseIter.cnct) {
		return FALSE;
	}

	iter->stamp = self->stamp;

	/*
	 * HACK: GTK's GtkTreeIter has 3 * sizeof(void*) of user data space,
	 * which we directly use as cheap storage
	 */
	*USER_DATA_2_BASE_ITER(iter->user_data) = baseIter;

	return TRUE;
}


/*
 * Returns TRUE or FALSE depending on whether
 * the row specified by 'iter' has any children.
 * We only have a list and thus no children.
 */
static gboolean gbase_list_iter_has_child(GtkTreeModel *tree_model, GtkTreeIter *iter) {
	(void)(tree_model); // unused
	(void)(iter);       // unused

	return FALSE;
}


/*
 * Returns the number of children the row
 * specified by 'iter' has. This is usually 0,
 * as we only have a list and thus do not have
 * any children to any rows. A special case is
 * when 'iter' is NULL, in which case we need
 * to return the number of top-level nodes,
 * ie. the number of rows in our list.
 */
static gint gbase_list_iter_n_children(GtkTreeModel *tree_model, GtkTreeIter *iter) {
	g_return_val_if_fail(GBASE_IS_LIST(tree_model), -1);

	GBaseList *self = GBASE_LIST(tree_model);

	if (!iter) {
		return base_get_count(self->base_ctx);
	}

	return 0;
}


/*
 * If the row specified by 'parent' has any
 * children, set 'iter' to the n-th child and
 * return TRUE if it exists, otherwise FALSE.
 * A special case is when 'parent' is NULL, in
 * which case we need to set 'iter' to the n-th
 * row if it exists.
 */
static gboolean gbase_list_iter_nth_child(GtkTreeModel *tree_model, GtkTreeIter *iter, GtkTreeIter *parent, gint idx) {
	g_return_val_if_fail(GBASE_IS_LIST(tree_model), FALSE);
	g_return_val_if_fail(iter != NULL, FALSE);

	if (parent) {
		return FALSE;
	}

	GBaseList *self = GBASE_LIST(tree_model);
	base_iter_t baseIter = base_get_nth(self->base_ctx, idx);

	if (!baseIter.cnct) {
		return FALSE;
	}

	iter->stamp = self->stamp;

	/*
	 * HACK: GTK's GtkTreeIter has 3 * sizeof(void*) of user data space,
	 * which we directly use as cheap storage
	 */
	*USER_DATA_2_BASE_ITER(iter->user_data) = baseIter;

	return TRUE;
}


/*
 * Point 'iter' to the parent node of 'child'. As
 * we have a list and thus no children and no
 * parents of children, we can just return FALSE.
 */
static gboolean gbase_list_iter_parent(GtkTreeModel *tree_model, GtkTreeIter *iter, GtkTreeIter *child) {
	(void)(tree_model); // unused
	(void)(iter);       // unused
	(void)(child);      // unused

	return FALSE;
}



/*
 * here we register our new type and its interfaces
 * with the type system. If you want to implement
 * additional interfaces like GtkTreeSortable, you
 * will need to do it here.
 */
GType gbase_list_get_type() {
	static GType gbase_list_type = 0;

	/* Some boilerplate type registration stuff */
	if (gbase_list_type == 0) {
		static const GTypeInfo gbase_list_info = {
			sizeof(GBaseListClass),
			NULL,                                  /* base_init */
			NULL,                                  /* base_finalize */
			(GClassInitFunc)gbase_list_class_init,
			NULL,                                  /* class finalize */
			NULL,                                  /* class_data */
			sizeof(GBaseList),
			0,                                     /* n_preallocs */
			(GInstanceInitFunc)gbase_list_init,
			NULL,
		};

		static const GInterfaceInfo tree_model_info = {
			(GInterfaceInitFunc)gbase_list_tree_model_init,
			NULL,
			NULL,
		};

		/* First register the new derived type with the GObject type system */
		gbase_list_type = g_type_register_static(G_TYPE_OBJECT, "GBaseList", &gbase_list_info, 0);

		/* Now register our GtkTreeModel interface with the type system */
		g_type_add_interface_static(gbase_list_type, GTK_TYPE_TREE_MODEL, &tree_model_info);
	}

	return gbase_list_type;
}

GBaseList *gbase_list_new(base_t *ctx) {
	g_return_val_if_fail(ctx != NULL, FALSE);

	GBaseList *list = (GBaseList *)g_object_new(GBASE_TYPE_LIST, NULL);
	g_assert(list != NULL);

	base_prepend_placeholder(ctx);

	list->base_ctx = ctx;

	return list;
}

static void gbase_list_row_prepended(GBaseList *self) {
	GtkTreePath *path = gtk_tree_path_new();
	gtk_tree_path_append_index(path, 0); // base_prepend() inserts at head => position 0

	GtkTreeIter iter;
	gbase_list_get_iter(GTK_TREE_MODEL(self), &iter, path);
	gtk_tree_model_row_inserted(GTK_TREE_MODEL(self), path, &iter);

	gtk_tree_path_free(path);
}

void gbase_list_prepend_record(GBaseList *self, const gchar *number, const gchar *name, const gchar *fname) {
	g_return_if_fail(GBASE_IS_LIST(self));

	if (base_prepend(self->base_ctx, number, name, fname)) {
		gbase_list_row_prepended(self);
	}
}

bool gbase_list_change_record(GBaseList *self, const gchar *value, unsigned int row, unsigned int col) {
	g_return_val_if_fail(GBASE_IS_LIST(self), false);
	g_return_val_if_fail(col < BASE_COL_COUNT, false);

	bool ret = false;

	GtkTreePath *path = gtk_tree_path_new();
	gtk_tree_path_append_index(path, row);

	GtkTreeIter iter;
	gbase_list_get_iter(GTK_TREE_MODEL(self), &iter, path);

	base_entry_t *entry = base_iter_value(USER_DATA_2_BASE_ITER(iter.user_data));

	if (entry && base_entry_change(self->base_ctx, entry, value, col)) {
		gtk_tree_model_row_changed(GTK_TREE_MODEL(self), path, &iter);

		if (row == 0 && entry->number.len && entry->name.len && entry->fname.len) {
			base_prepend_placeholder(self->base_ctx);
			gbase_list_row_prepended(self);
			ret = true;
		}
	}

	gtk_tree_path_free(path);

	return ret;
}

void gbase_list_delete_record(GBaseList *self, GtkTreeIter *iter) {
	g_return_if_fail(GBASE_IS_LIST(self));

	GtkTreePath *path = gbase_list_get_path(GTK_TREE_MODEL(self), iter);
	base_entry_t *entry = base_iter_value(USER_DATA_2_BASE_ITER(iter->user_data));

	if (entry && base_entry_remove(self->base_ctx, entry)) {
		gtk_tree_model_row_deleted(GTK_TREE_MODEL(self), path);
	}

	gtk_tree_path_free(path);
}
