#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL.h>
#include "src/play.h"

typedef struct Logiciel
{
    GtkBuilder *builder;
    GtkWidget *window;
    GtkWidget *playlist_window;
    const gchar *username;
    GtkWidget *create_panel;
    GtkListStore *list;
} Logiciel;


void load_playlists(const char *filename, GtkListStore *liststore) {
    gtk_list_store_clear(liststore);
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        g_warning("Could not open file: %s", filename);
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Retirer la nouvelle ligne
        line[strcspn(line, "\n")] = '\0';

        // Diviser la ligne au caractère '~'
        char *token = strtok(line, "~");
        if (token != NULL) {
            GtkTreeIter iter;
            gtk_list_store_append(liststore, &iter);
            gtk_list_store_set(liststore, &iter, 0, token, -1);
        }
    }

    fclose(file);
}

void load_playlist(const char *path, GtkListStore *liststore) {
    gtk_list_store_clear(liststore);

    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(path)) != NULL) {
        /* parcourir tous les fichiers et répertoires dans le dossier */
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_type == DT_REG) { // Seulement les fichiers réguliers
                char *filename = ent->d_name;
                size_t len = strlen(filename);
                if (len > 4 && strcmp(filename + len - 4, ".mp3") == 0) {
                    // Si le fichier a l'extension .mp3, ajoutez-le sans l'extension à la liste
                    filename[len - 4] = '\0'; // Supprimer l'extension .mp3
                    GtkTreeIter iter;
                    gtk_list_store_append(liststore, &iter);
                    gtk_list_store_set(liststore, &iter, 0, filename, -1);
                }
            }
        }
        closedir(dir);
    } else {
        /* could not open directory */
        g_warning("Could not open directory: %s", path);
    }
}

void on_playlist_window_destroy(GtkWidget *widget, gpointer user_data) {
    Logiciel *logiciel = user_data;
    g_signal_handlers_disconnect_by_func(widget, G_CALLBACK(on_playlist_window_destroy), logiciel);
    logiciel->playlist_window = NULL;
}


void create_playlist(GtkWidget *widget, gpointer user_data)
{
    Logiciel *logiciel = user_data;
    g_print("create button clicked\n");
    GtkEntry *name_entry = GTK_ENTRY(gtk_builder_get_object(logiciel->builder, "playlist_name"));
    GtkEntry *path_entry = GTK_ENTRY(gtk_builder_get_object(logiciel->builder, "path_entry"));
    const gchar *name = gtk_entry_get_text(name_entry);
    const gchar *path = gtk_entry_get_text(path_entry);
    if (strcmp(name, "") != 0 || strcmp(path, "") != 0)
    {
        FILE *file = fopen("src/playlists", "a");
        if (file == NULL) {
            g_warning("Could not open file to write playlist.");
            return;
        }
        gchar towrite[256];
        strcpy(towrite, name);
        strcat(towrite, "~");
        strcat(towrite, path);
        fprintf(file, "%s\n", towrite);
        fclose(file);
        gtk_widget_destroy(logiciel->create_panel);
    }

    //logiciel->list = gtk_list_store_new(1, G_TYPE_STRING);
    load_playlists("src/playlists", logiciel->list);
    gtk_widget_show_all(logiciel->window);



}


void browse_button_callback(GtkButton *button, gpointer user_data)
{
    g_print("Browse button clicked\n");

    Logiciel *logiciel = user_data;
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Choisir un dossier",
                        GTK_WINDOW(gtk_builder_get_object(logiciel->builder, "main_window")),
                        GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                        "Annuler",
                        GTK_RESPONSE_CANCEL,
                        "Choisir",
                        GTK_RESPONSE_ACCEPT,
                        NULL);

    if (dialog == NULL) {
        g_warning("Failed to create file chooser dialog.");
        return;
    }

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        gchar *foldername = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        gtk_widget_destroy(dialog);

        GtkEntry *entry = GTK_ENTRY(gtk_builder_get_object(logiciel->builder, "path_entry"));
        if (entry) {
            gtk_entry_set_text(entry, foldername);
        } else {
            g_warning("Failed to get GtkEntry from builder.");
        }

        g_free(foldername);
    } else {
        gtk_widget_destroy(dialog);
    }
}


void ok_name_callback(GtkWidget *widget, gpointer user_data)
{
    Logiciel *logiciel = user_data;
    GtkEntry *entry = GTK_ENTRY(gtk_builder_get_object(logiciel->builder, "name_entry"));
    GtkLabel *label = GTK_LABEL(gtk_builder_get_object(logiciel->builder, "current_username"));

    if (!entry) {
        g_warning("Failed to get GtkEntry from builder.");
        return;
    }
    if (!label){
        g_warning("Failed to get GtkLabel from builder.");
        return;
    }

    const gchar *username = gtk_entry_get_text(entry);
    logiciel->username = username;

    if (strlen(username) < 5) {
        GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Username must be at least 5 characters long.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    } else {
        FILE *file = fopen("src/username", "w");
        if (file == NULL) {
            g_warning("Could not open file to write username.");
            return;
        }
        fprintf(file, "%s\n", username);
        fclose(file);

        gtk_label_set_text(label, username);
    }
}




void create_panel_display(GtkWidget *widget, gpointer user_data)
{
    Logiciel *logiciel = user_data;

    GtkWidget *create_panel = logiciel->create_panel;

    gtk_widget_show_all(create_panel);

    // Connecter les signaux à chaque fois que create_panel est affiché
    GtkButton *browse_btn = GTK_BUTTON(gtk_builder_get_object(logiciel->builder, "browse_button"));
    g_signal_connect(browse_btn, "clicked", G_CALLBACK(browse_button_callback), logiciel);
 
    GtkButton *create_create_btn = GTK_BUTTON(gtk_builder_get_object(logiciel->builder, "create_create"));
    g_signal_connect(create_create_btn, "clicked", G_CALLBACK(create_playlist), logiciel);


}

void on_tracks_treeview_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data) 
{
    Logiciel *logiciel = user_data;
    GtkTreeModel *model = gtk_tree_view_get_model(treeview);
    GtkTreeIter iter;
    gtk_tree_model_get_iter(model, &iter, path);

    gchar *track_name;
    gtk_tree_model_get(model, &iter, 0, &track_name, -1);

    g_print("Track selected: %s\n", track_name);
    //play(track_name, logiciel);
    g_free(track_name);
}


void open_playlist_window(gchar *playlist_name, Logiciel *logiciel) {

    /*GtkWidget *playlist_window = logiciel->playlist_window;
    gtk_widget_show_all(playlist_window);

    
    GtkButton* normal_play = GTK_BUTTON(gtk_builder_get_object(logiciel->builder, "normal_play"));
    g_signal_connect(normal_play, "clicked", G_CALLBACK(normal_play_button_clicked), logiciel);
    
    
    GtkTreeView *treeview = GTK_TREE_VIEW(gtk_builder_get_object(logiciel->builder, "inside_playlist"));
    
    GtkListStore *liststore = gtk_list_store_new(1, G_TYPE_STRING);
    gtk_tree_view_set_model(treeview, GTK_TREE_MODEL(liststore));

    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("Tracks", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(treeview, column);

    load_playlist(path, liststore);
    g_signal_connect(treeview, "row-activated", G_CALLBACK(on_tracks_treeview_row_activated), logiciel);*/
    if (logiciel->playlist_window == NULL) {
        logiciel->playlist_window = GTK_WIDGET(gtk_builder_get_object(logiciel->builder, "playlist_window"));
        if (!logiciel->playlist_window) {
            g_warning("Failed to get playlist_window from builder");
            return;
        }

        g_signal_connect(logiciel->playlist_window, "destroy", G_CALLBACK(on_playlist_window_destroy), logiciel);
    }

    gtk_widget_show_all(logiciel->playlist_window);

    GtkButton *normal_play = GTK_BUTTON(gtk_builder_get_object(logiciel->builder, "normal_play"));
    g_signal_connect(normal_play, "clicked", G_CALLBACK(normal_play_button_clicked), logiciel);

}

void open_playlist(gchar *playlist_name, Logiciel *logiciel) {
    FILE *file = fopen("src/playlists", "r");
    if (file == NULL) {
        g_warning("Could not open file: src/playlists");
        return;
    }

    char line[256];
    gchar *path = NULL;

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0'; // Supprimer le caractère de nouvelle ligne

        gchar *name = strtok(line, "~");
        gchar *temp_path = strtok(NULL, "~");

        if (name != NULL && temp_path != NULL && strcmp(name, playlist_name) == 0) {
            path = g_strdup(temp_path); 
            break;
        }
    }

    g_print("%s\n", path);
    GtkTreeView *treeview = GTK_TREE_VIEW(gtk_builder_get_object(logiciel->builder, "inside_playlist"));

    GtkListStore *liststore = gtk_list_store_new(1, G_TYPE_STRING);
    gtk_tree_view_set_model(treeview, GTK_TREE_MODEL(liststore));

    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("Playlists", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(treeview, column);
    g_signal_connect(treeview, "row-activated", G_CALLBACK(on_tracks_treeview_row_activated), logiciel);

    load_playlist(path, liststore);
    open_playlist_window(playlist_name, logiciel);
    //open_playlist_window(playlist_name, logiciel, path);
    g_free(path);


    fclose(file);
}

void on_playlist_treeview_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data) {
    Logiciel *logiciel = user_data;
    GtkTreeModel *model = gtk_tree_view_get_model(treeview);
    GtkTreeIter iter;
    gtk_tree_model_get_iter(model, &iter, path);

    gchar *playlist_name;
    gtk_tree_model_get(model, &iter, 0, &playlist_name, -1);

    g_print("Playlist selected: %s\n", playlist_name);
    open_playlist(playlist_name, logiciel);
    g_free(playlist_name);
}

int main(int argc, char *argv[])
{
    GtkWidget *window;
    GtkBuilder *builder;

    Logiciel logiciel;

    // Initialisation de GTK
    gtk_init(NULL, NULL);

    // Initialisation du GtkBuilder
    builder = gtk_builder_new();
    logiciel.builder = builder;

    gtk_builder_add_from_file(builder, "music-player.glade", NULL);

    // Obtention de la référence à la fenêtre principale
    window = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
    logiciel.window = window;
    logiciel.create_panel = GTK_WIDGET(gtk_builder_get_object(logiciel.builder, "create_panel"));
    /*logiciel.playlist_window = GTK_WIDGET(gtk_builder_get_object(logiciel.builder, "playlist_window"));
    if (!logiciel.playlist_window) {
        g_warning("Failed to get playlist_window from builder");
        return 1;
    }*/
    logiciel.playlist_window = NULL;
    GtkButton* create_button = GTK_BUTTON(gtk_builder_get_object(builder, "create_playlist"));
    g_signal_connect(create_button, "clicked", G_CALLBACK(create_panel_display), &logiciel);


    GtkButton* ok_name = GTK_BUTTON(gtk_builder_get_object(builder, "ok_name"));
    g_signal_connect(ok_name, "clicked", G_CALLBACK(ok_name_callback), &logiciel);

    GtkTreeView *treeview = GTK_TREE_VIEW(gtk_builder_get_object(builder, "playlist_treeview"));


    GtkListStore *liststore = gtk_list_store_new(1, G_TYPE_STRING);
    gtk_tree_view_set_model(treeview, GTK_TREE_MODEL(liststore));

    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("Playlists", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(treeview, column);

    logiciel.list = liststore;
    load_playlists("src/playlists", liststore);

    g_signal_connect(treeview, "row-activated", G_CALLBACK(on_playlist_treeview_row_activated), &logiciel);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}