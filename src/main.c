#include <gtk/gtk.h>

enum {
    FILE_COLUMN,
    LOCKED_COLUMN,
    N_COLUMNS
};

static void callback(GtkWidget *widget, gpointer data) {
    g_print ("Hello again - %s was pressed\n", (gchar *) data);
}

static gboolean delete_event(GtkWidget *widget, GdkEvent *event, gpointer data) {
    gtk_main_quit ();
    return FALSE;
}

static void tree_selection_changed_cb(GtkTreeSelection *selection, gpointer data) {
    GtkTreeIter iter;
    GtkTreeModel *model;
    gchar *file;

    if(gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter, FILE_COLUMN, &file, -1);
        g_print("You selected a file %s\n", file);
        g_free(file);
    }
}

int main(int argc, char *argv[]) {
    GtkWidget *window;
    GtkWidget *button;
    GtkWidget *box1;

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gtk_window_set_title(GTK_WINDOW(window), "hart");

    g_signal_connect(window, "delete-event", G_CALLBACK(delete_event), NULL);

    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    box1 = gtk_hbox_new(FALSE, 0);

    gtk_container_add(GTK_CONTAINER(window), box1);

    button = gtk_button_new_with_label("Button 1");
    g_signal_connect(button, "clicked", G_CALLBACK(callback), (gpointer)"button 1");
    gtk_box_pack_start(GTK_BOX(box1), button, TRUE, TRUE, 0);
    gtk_widget_show(button);

    button = gtk_button_new_with_label("Button 2");
    g_signal_connect(button, "clicked", G_CALLBACK(callback), (gpointer)"button 2");

    gtk_box_pack_start(GTK_BOX(box1), button, TRUE, TRUE, 0);

    //Create tree model
    GtkTreeStore* store = gtk_tree_store_new(N_COLUMNS,
            G_TYPE_STRING,
            G_TYPE_BOOLEAN);
    //Get an iterator to the root of the tree and start adding data.
    GtkTreeIter iter;
    GtkTreeIter child_iter;
    gtk_tree_store_append(store, &iter, NULL);
    gtk_tree_store_set(store, &iter, 
            FILE_COLUMN, "test_file0",
//            LOCKED_COLUMN, TRUE,
            -1);
    gtk_tree_store_append(store, &child_iter, &iter);
    gtk_tree_store_set(store, &child_iter, 
            FILE_COLUMN, "test_child_file0",
            LOCKED_COLUMN, TRUE,
            -1);
    //Create the tree's view
    GtkWidget *tree;
    tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL (store));
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("File",
            renderer,
            "text", FILE_COLUMN,
            NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

    renderer = gtk_cell_renderer_toggle_new();
    column = gtk_tree_view_column_new_with_attributes("Locked",
            renderer,
            "active", LOCKED_COLUMN,
            NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
    //Add ui callbacks for the tree
    GtkTreeSelection *select;
    select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
    gtk_tree_selection_set_mode(select, GTK_SELECTION_SINGLE);
    g_signal_connect(G_OBJECT(select), "changed",
            G_CALLBACK(tree_selection_changed_cb),
            NULL);
    //Add tree to box
    gtk_box_pack_start(GTK_BOX(box1), tree, TRUE, TRUE, 0);

    //Show all of the widgets
    gtk_widget_show(tree);
    gtk_widget_show(button);
    gtk_widget_show(box1);
    gtk_widget_show(window); //Show window last so they all appear at once.

    gtk_main ();

    return 0;
}
