#include <gtk/gtk.h>

enum {
    FILE_COLUMN,
    LOCKED_COLUMN,
    N_COLUMNS
};

static GtkTreeStore* store;

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

void print_file_tree(const gchar* dir_name) {
    GDir* dir;
    GError* error;
    const gchar* filename;

    printf("print_file_tree: %s\n", dir_name);
    dir = g_dir_open(dir_name, 0, &error);
    while((filename = g_dir_read_name(dir))) {
        //Find all of the directories first of all:
        GFileTest test_mask = G_FILE_TEST_IS_DIR;
        gchar* new_filename = g_strjoin("/", dir_name, filename, NULL);
        if(g_file_test(new_filename, test_mask)) {
            printf("DIR: %s\n", filename);
            print_file_tree(new_filename);
            g_free(new_filename);
        } else {
            printf("FILE: %s\n", filename);
        }
    }
}

void create_file_tree(const gchar* root_dir_name, GtkTreeIter* root_iter) {
    GDir* dir;
    GError* error;
    const gchar* filename;
    GtkTreeIter child_iter;

    dir = g_dir_open(root_dir_name, 0, &error);
    while((filename = g_dir_read_name(dir))) {
        //Find all of the directories first of all:
        GFileTest test_mask = G_FILE_TEST_IS_DIR;
        gchar* new_filename = g_strjoin("/", root_dir_name, filename, NULL);
        if(g_file_test(new_filename, test_mask)) {
            gtk_tree_store_append(store, &child_iter, root_iter);
            gtk_tree_store_set(store, &child_iter, 
                    FILE_COLUMN, new_filename,
                    -1);
            create_file_tree(new_filename, &child_iter);
            g_free(new_filename);
        } else {
            //Add to the root normally.
            gtk_tree_store_append(store, &child_iter, root_iter);
            gtk_tree_store_set(store, &child_iter, 
                    FILE_COLUMN, filename,
                    LOCKED_COLUMN, TRUE,
                    -1);
        }
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

    //Create tree model
    store = gtk_tree_store_new(N_COLUMNS,
            G_TYPE_STRING,
            G_TYPE_BOOLEAN);
    create_file_tree(".", NULL);
    //Sort model
    GtkTreeModel* sorted_model = gtk_tree_model_sort_new_with_model(store);
    gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(sorted_model),
            FILE_COLUMN, GTK_SORT_ASCENDING);

    //Create the tree's view
    GtkWidget *tree;
    //tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(sorted_model));

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

    //Read files in and print them as a test
    print_file_tree(".");

    //Show all of the widgets
    gtk_widget_show(tree);
    gtk_widget_show(box1);
    gtk_widget_show(window); //Show window last so they all appear at once.

    gtk_main ();

    return 0;
}
