/*
 * Name: Leonard Hecker
 * Studiengruppe: 13/041/61
 * MatrNr: XXXXX
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gbase-list-private.h"


static GBaseList *gbase_list_singleton;
static GtkWidget *tree_view_singleton;


/*
 * For testing purposes in create_table_using_base()
 * Fills the internal list with static test data.
 * phonebook.dat should be deleted before use.
 */
void fill_model(GBaseList *list) {
	const gchar  *firstnames[] = { "Joe", "Jane", "William", "Hannibal", "Timothy", "Gargamel", NULL } ;
	const gchar  *surnames[]   = { "Grokowich", "Twitch", "Borheimer", "Bork", NULL } ;
	const gchar **fname, **sname;

	for (sname = surnames; *sname != NULL; sname++) {
		for (fname = firstnames; *fname != NULL; fname++) {
			gchar *number = g_strdup_printf("0172%04u%04u", rand() % 10000, rand() % 10000);
			gbase_list_prepend_record(list, number, *sname, *fname);
			g_free(number);
		}
	}
}


void on_edited(GtkCellRendererText *renderer, gchar *path, gchar *new_text, int col) {
	(void)(renderer); // unused

	bool addedPlaceholder = gbase_list_change_record(gbase_list_singleton, new_text, atoi(path), col);

	if (addedPlaceholder) {
		gtk_tree_view_scroll_to_point(GTK_TREE_VIEW(tree_view_singleton), -1, 0);
	}
}

gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
	(void)(user_data); // unused

	switch (event->keyval) {
		case GDK_KEY_Delete:
		{
			GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
			GtkTreeIter iter;

			if (gtk_tree_selection_get_selected(selection, NULL, &iter)) {
				gbase_list_delete_record(gbase_list_singleton, &iter);
			}

			return TRUE;
		}
		case GDK_KEY_Tab:
		case GDK_KEY_ISO_Left_Tab:
		{
			GtkTreePath *path;
			GtkTreeViewColumn *focus_column;

			gtk_tree_view_get_cursor(GTK_TREE_VIEW(widget), &path, &focus_column);

			if (path) {
				GList *columns = gtk_tree_view_get_columns(GTK_TREE_VIEW(widget));
				GList *currentColumn = g_list_find(columns, focus_column);
				GList *nextColumn = event->state & GDK_SHIFT_MASK ? g_list_previous(currentColumn) : g_list_next(currentColumn);

				if (nextColumn) {
					gtk_tree_view_set_cursor(GTK_TREE_VIEW(widget), path, nextColumn->data, TRUE);
				}
			}

			return TRUE;
		}
		default:
			return FALSE;
	}
}

void append_column(gint col, gchar *title) {
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	GtkTreeViewColumn *column = gtk_tree_view_column_new();

	g_object_set(renderer, "editable", TRUE, NULL);
	g_signal_connect(renderer, "edited", G_CALLBACK(on_edited), GINT_TO_POINTER(col));

	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_add_attribute(column, renderer, "text", col);
	gtk_tree_view_column_set_title(column, title);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view_singleton), column);
}

int main(int argc, char *argv[]) {
	gtk_init(&argc, &argv);

	gchar *dirname = g_path_get_dirname(argv[0]);
	gchar *dataFilename = g_build_filename(dirname, "phonebook.dat", NULL);
	base_t *ctx = base_new(dataFilename);
	g_free(dirname);
	g_free(dataFilename);


	GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "PSP1Beleg");
	gtk_window_set_default_size(GTK_WINDOW(window), 640, 480);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	g_signal_connect_swapped(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);


	gbase_list_singleton = gbase_list_new(ctx);
	tree_view_singleton = gtk_tree_view_new_with_model(GTK_TREE_MODEL(gbase_list_singleton));
	g_object_unref(gbase_list_singleton);

	//fill_model(gbase_list_singleton);

	gtk_tree_view_set_search_column(GTK_TREE_VIEW(tree_view_singleton), GBASE_LIST_COL_NUMBER);
	g_signal_connect(G_OBJECT(tree_view_singleton), "key-press-event", G_CALLBACK(on_key_press), NULL);

	append_column(GBASE_LIST_COL_NUMBER,   "Phone Number");
	append_column(GBASE_LIST_COL_NAME,     "Surname");
	append_column(GBASE_LIST_COL_FORENAME, "Forename");


	GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scrolled_window), tree_view_singleton);
	gtk_container_add(GTK_CONTAINER(window), scrolled_window);


	gtk_widget_show_all(window);
	gtk_main();


	bool ok = base_save(ctx);
	base_delete(ctx);

	return ok ? 0 : 1;
}
