#include <gtk/gtk.h>

enum {
    FILE_COLUMN,
    LOCKED_COLUMN,
    N_COLUMNS
};

GtkTreeStore* store;
GtkTreeModel* sorted_model;
GtkWidget* tree;
GtkWidget* filew;
GtkWidget *window;

static void callback(GtkWidget *widget, gpointer data) {
    g_print ("Hello again - %s was pressed\n", (gchar *) data);
}

static gboolean delete_event(GtkWidget *widget, GdkEvent *event, gpointer data) {
    gtk_main_quit();
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

/*  Get the selected filename and print it to the console */
static void file_ok_sel(GtkWidget *w, GtkFileSelection *fs)
{
    g_print ("%s\n", gtk_file_selection_get_filename(GTK_FILE_SELECTION(fs)));
    store = gtk_tree_store_new(N_COLUMNS,
            G_TYPE_STRING,
            G_TYPE_BOOLEAN);
    create_file_tree(gtk_file_selection_get_filename(GTK_FILE_SELECTION(fs)), NULL);
    sorted_model = gtk_tree_model_sort_new_with_model(GTK_TREE_MODEL(store));
    gtk_tree_view_set_model(GTK_TREE_VIEW(tree), GTK_TREE_MODEL(sorted_model));
    gtk_widget_destroy(filew);
}

//static void file_button_callback(GtkWidget *widget, gpointer data) {
static void file_button_callback(GtkFileChooserButton *widget, gpointer data) {
    g_print("Hello again - %s was pressed\n", (char *) data);
    g_print("dir is %s\n", gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget)));

    store = gtk_tree_store_new(N_COLUMNS,
            G_TYPE_STRING,
            G_TYPE_BOOLEAN);
    create_file_tree(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget)), NULL);
    sorted_model = gtk_tree_model_sort_new_with_model(GTK_TREE_MODEL(store));
    gtk_tree_view_set_model(GTK_TREE_VIEW(tree), GTK_TREE_MODEL(sorted_model));
/*
    filew = gtk_file_selection_new("File selection");
    gtk_file_chooser_set_action(filew, GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);

    g_signal_connect(GTK_FILE_SELECTION(filew)->ok_button,
            "clicked", G_CALLBACK(file_ok_sel), (gpointer)filew);
    g_signal_connect_swapped(GTK_FILE_SELECTION(filew)->cancel_button,
            "clicked", G_CALLBACK(gtk_widget_destroy),
            filew);
    gtk_widget_show(filew);
*/

/*
    GtkWidget *dialog;
    dialog = gtk_file_chooser_dialog_new("Set Root Directory",
            window,
            GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
            NULL);
*/
/*
    GtkWidget *dialog;
    g_object_get(widget,
            "dialog", dialog,
            NULL);

    if(gtk_dialog_run(GTK_DIALOG(widget)) == GTK_RESPONSE_ACCEPT) {
        char *filename;

        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget));

        store = gtk_tree_store_new(N_COLUMNS,
                G_TYPE_STRING,
                G_TYPE_BOOLEAN);
        create_file_tree(gtk_file_selection_get_filename(filename), NULL);
        sorted_model = gtk_tree_model_sort_new_with_model(GTK_TREE_MODEL(store));
        gtk_tree_view_set_model(GTK_TREE_VIEW(tree), GTK_TREE_MODEL(sorted_model));

        g_free (filename);
    }
*/
}

int main(int argc, char *argv[]) {
    GtkWidget *button;
    GtkWidget *box1;

    gtk_init(&argc, &argv);


    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gtk_window_set_title(GTK_WINDOW(window), "hart");

    g_signal_connect(window, "delete-event", G_CALLBACK(delete_event), NULL);

    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    box1 = gtk_vbox_new(FALSE, 0);

    gtk_container_add(GTK_CONTAINER(window), box1);

    //Create tree model
    store = gtk_tree_store_new(N_COLUMNS,
            G_TYPE_STRING,
            G_TYPE_BOOLEAN);
    create_file_tree(".", NULL);
    //Sort the model for convenience
    sorted_model = gtk_tree_model_sort_new_with_model(GTK_TREE_MODEL(store));
    gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(sorted_model),
            FILE_COLUMN, GTK_SORT_ASCENDING);

    //Create the tree's view
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

    //Create console output label
    GtkWidget* frame = gtk_frame_new("Console output");
    //GtkWidget* label = gtk_label_new("hg locks");
    GtkWidget* text_view = gtk_text_view_new();
    //gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
    //gtk_container_add(GTK_CONTAINER(frame), label);
    GtkTextTagTable* text_tag_table = gtk_text_tag_table_new();
    GtkTextBuffer* console_buffer = gtk_text_buffer_new(text_tag_table);
    gtk_text_view_set_buffer(GTK_TEXT_VIEW(text_view), console_buffer);
    gchar* test_str = "blah\nwoiajheoih\nwoeihoiwha\nweoihewoih\nwoeihoi";
    gtk_text_buffer_set_text(console_buffer, 
            test_str,
            strlen(test_str));
    //gtk_text_view_set_buffer("HI");
    gtk_container_add(GTK_CONTAINER(frame), text_view);
    gtk_box_pack_start(GTK_BOX(box1), frame, TRUE, TRUE, 0);

    //Create button w/ label
/*
    GtkWidget* file_button = gtk_button_new_with_label("Choose a folder");
    gtk_box_pack_start(GTK_BOX(box1), file_button, TRUE, TRUE, 0);

    g_signal_connect(file_button, "clicked", G_CALLBACK(file_button_callback), (gpointer)"file_button");
*/

    GtkWidget* file_button = gtk_file_chooser_button_new("Select a folder",
            GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
    gtk_box_pack_start(GTK_BOX(box1), file_button, TRUE, TRUE, 0);
    g_signal_connect(file_button, "file-set", G_CALLBACK(file_button_callback), (gpointer)"file_button");

    //Show all of the widgets
    gtk_widget_show(file_button);
    gtk_widget_show(tree);
    gtk_widget_show(frame);
    gtk_widget_show(text_view);
    gtk_widget_show(box1);
    gtk_widget_show(window); //Show window last so they all appear at once.

    gtk_main ();

    return 0;
}
