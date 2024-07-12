#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL.h>
#include "play.h"
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <time.h>



typedef struct Play_song
{
    gchar *track_name;
    gchar *playlist_path;
    pthread_t thread_id;
    pid_t child_pid;
    bool should_terminate;
    pthread_mutex_t mutex;
} Play_song;


typedef struct Logiciel
{
    GtkBuilder *builder;
    GtkWidget *window;
    GtkWidget *playlist_window;
    const gchar *username;
    GtkWidget *create_panel;
    GtkListStore *list;
    GtkListStore *tracks;
    gchar *playlist_name;
    gchar *playlist_path;
    GtkLabel *label2;
    int test;
    Play_song *playcurrent;
} Logiciel;



void terminate_audio_process(pid_t pid) {
    if (pid > 0) {
        kill(pid, SIGTERM);
        waitpid(pid, NULL, 0);
    }
}


void on_window_close(GtkWidget *widget, gpointer user_data) {
    Logiciel *logiciel = user_data;
    // Terminer le processus de lecture audio en cours avant de fermer GTK
    if (logiciel->playcurrent != NULL && logiciel->playcurrent->child_pid > 0) {
        terminate_audio_process(logiciel->playcurrent->child_pid);
        //g_free(logiciel->playcurrent->track_name);
        //g_free(logiciel->playcurrent->playlist_path);
        //g_free(logiciel->playcurrent);
        logiciel->playcurrent = NULL;
    }
    gtk_main_quit();
}

gboolean on_window_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
    gtk_widget_hide(widget);
    Logiciel *logiciel = user_data;
    // Terminer le processus de lecture audio en cours avant de fermer GTK
    if (logiciel->playcurrent != NULL && logiciel->playcurrent->child_pid > 0) {
        terminate_audio_process(logiciel->playcurrent->child_pid);
        //g_free(logiciel->playcurrent->track_name);
        //g_free(logiciel->playcurrent->playlist_path);
        //g_free(logiciel->playcurrent);
        logiciel->playcurrent = NULL;
    }
    return TRUE; // Empêche la destruction de la fenêtre
}



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

void load_playlist(const char *path, Logiciel *logiciel) {
    
    GtkListStore *liststore = logiciel->tracks;
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
        gtk_widget_hide(logiciel->create_panel);

    }
    gchar command[512];
    snprintf(command, sizeof(command), "python3 src/analyse.py \"%s\" \"%s\"", path, name);
    // Ouvrir un flux de fichier pour capturer la sortie du script Python
    FILE *pipe = popen(command, "r");
    if (pipe == NULL) {
        g_warning("Failed to open pipe to Python script.");
        return;
    }
    // Lire et traiter la sortie ligne par ligne si nécessaire
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        // Traiter ou ignorer la sortie selon vos besoins
        printf("Python output: %s", buffer); // Exemple d'affichage dans la console C
    }
    // Fermer le flux de fichier
    pclose(pipe);

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


void delete_playlist_callback(GtkWidget *widget, gpointer user_data)
{
    Logiciel *logiciel = user_data;
    const char *playlist_name = logiciel->playlist_name;
    const char *filename = "src/playlists";
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Could not open file");
        return;
    }

    FILE *temp_file = fopen("temp_playlists", "w");
    if (temp_file == NULL) {
        perror("Could not open temporary file");
        fclose(file);
        return;
    }

char line[500];
while (fgets(line, sizeof(line), file)) {
    // Retirer la nouvelle ligne
    line[strcspn(line, "\n")] = '\0';
    char *copy_of_line = strdup(line);
    // Diviser la ligne au caractère '~'
    const char *name = strtok(line, "~");
    // Vérifier si le nom correspond à playlist_name
    if (strcmp(name, playlist_name) == 0) {
        // Si le nom correspond, ne pas copier cette ligne dans le fichier temporaire
        continue;
    }
    fprintf(temp_file, "%s\n", copy_of_line);
}

    fclose(file);
    fclose(temp_file);

    if (remove(filename) != 0) {
        perror("Could not remove original file");
        return;
    }
    if (rename("temp_playlists", filename) != 0) {
        perror("Could not rename temporary file");
        return;
    }
    char new_path[256];
    snprintf(new_path, sizeof(new_path), "src/tracksdata/%s", playlist_name);
    remove(new_path);
    
    load_playlists("src/playlists", logiciel->list);
    gtk_widget_hide(logiciel->playlist_window);
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
}





void *play_thread(void *arg)
{
    Play_song *playsong = (Play_song *)arg;
    pid_t pid = fork();

    if (pid == 0) {
        gchar command[512];
        snprintf(command, sizeof(command), "python3 src/playsong.py \"%s\" \"%s\"", playsong->playlist_path, playsong->track_name);
        
        // Exécuter le script Python
        execlp("python3", "python3", "src/playsong.py", playsong->playlist_path, playsong->track_name, NULL);
        exit(0);
    } else if (pid > 0) {
        // Processus parent
        playsong->child_pid = pid;
        while (!playsong->should_terminate) {
            // Simulate doing some work
            usleep(100000); // Pause de 100ms pour éviter de surcharger le CPU
        }
        printf("Terminating the child process...\n");
        terminate_audio_process(playsong->child_pid);
    } else {
        // Erreur lors de la création du processus
        perror("fork");
        return NULL;
    }

    printf("Thread is terminating...\n");
    return NULL;
}






void playsong(gchar *playlist_path, gchar *track_name, Logiciel *logiciel)
{

    // Terminer la lecture audio en cours s'il y en a une
    if (logiciel->playcurrent != NULL && logiciel->playcurrent->child_pid > 0) {
        terminate_audio_process(logiciel->playcurrent->child_pid);
        //g_free(logiciel->playcurrent);
        logiciel->playcurrent = NULL;
    }

    // Démarrer la nouvelle lecture audio
    Play_song *playsong = g_malloc(sizeof(Play_song));
    playsong->track_name = g_strdup(track_name);
    playsong->playlist_path = g_strdup(playlist_path);
    playsong->should_terminate = false;
    logiciel->playcurrent = playsong;

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, play_thread, playsong);
    pthread_detach(thread_id); // Détache le thread pour éviter les fuites de mémoire
}



void on_tracks_treeview_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data) 
{
    Logiciel *logiciel = user_data;

    GtkTreeModel *model = gtk_tree_view_get_model(treeview);
    GtkTreeIter iter;
    gtk_tree_model_get_iter(model, &iter, path);
    gchar *track_name;
    gtk_tree_model_get(model, &iter, 0, &track_name, -1);

    FILE *file = fopen("src/playlists", "r");
    if (file == NULL) {
        g_warning("Could not open file: playlists");
        return;
    }

    char line[256];
    gchar *playlist_path = NULL;

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0'; // Supprimer le caractère de nouvelle ligne

        gchar *name = strtok(line, "~");
        gchar *temp_path = strtok(NULL, "~");

        if (name != NULL && temp_path != NULL && strcmp(name, logiciel->playlist_name) == 0) {
            playlist_path = g_strdup(temp_path); 
            break;
        }
    }

    fclose(file);

    if (playlist_path != NULL) {
        playsong(playlist_path, track_name, logiciel);
        g_free(playlist_path);
    } else {
        g_warning("Playlist path not found for %s", logiciel->playlist_name);
    }

    g_free(track_name);
}

void open_playlist_window(gchar *playlist_name, Logiciel *logiciel, gchar *path) {

    GtkWidget *playlist_window = logiciel->playlist_window;
    logiciel->playlist_path = path; 
    gtk_widget_show_all(playlist_window);
    load_playlist(path, logiciel);
}

void open_playlist(gchar *playlist_name, Logiciel *logiciel) {
    FILE *file = fopen("src/playlists", "r");
    if (file == NULL) {
        g_warning("Could not open file: playlists");
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

    logiciel->playlist_name = playlist_name;
    open_playlist_window(playlist_name, logiciel, path);
    //g_free(path);


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
    //g_free(playlist_name);
}

double get_mp3_duration(const char *filename) {
    char command[512];
    snprintf(command, sizeof(command), "python3 src/duration.py \"%s\"", filename);
    
    FILE *pipe = popen(command, "r");
    if (!pipe) {
        fprintf(stderr, "Could not open pipe for output.\n");
        return -1;
    }

    char buffer[128];
    fgets(buffer, sizeof(buffer), pipe);
    pclose(pipe);

    return atof(buffer);
}


void *sortedplaythread(void *arg)
{
    Logiciel *logiciel = (Logiciel *)arg;
    gchar filepath[256];
    snprintf(filepath, sizeof(filepath), "src/tracksdata/%s", logiciel->playlist_name);

    FILE *file = fopen(filepath, "r");
    if (file == NULL) {
        g_warning("Could not open file: %s", filepath);
        //return;
    }

    char line[256];
    gchar *track_name = NULL;

    // Read the first line and discard it
    if (fgets(line, sizeof(line), file) == NULL) {
        fclose(file);
        //return;
    }

    // Read the remaining lines
    while (fgets(line, sizeof(line), file)) {
        // Remove the newline character at the end of the line
        line[strcspn(line, "\n")] = '\0';
        // Get the track name before the "/"
        track_name = strtok(line, "/");
        size_t len = strlen(track_name);
        
        track_name[len - 4] = '\0';
        if (track_name != NULL) {
            // Call the playsong function with the appropriate arguments
            playsong(logiciel->playlist_path, track_name, logiciel);
            char *full_path = malloc(strlen(logiciel->playlist_path) + strlen("/") + strlen(track_name) + 1);
            snprintf(full_path, strlen(logiciel->playlist_path) + strlen("/") + strlen(track_name) + 1, "%s/%s", logiciel->playlist_path, track_name);
            char new_path[256];
            snprintf(new_path, sizeof(new_path), "%s/%s.mp3",logiciel->playlist_path , track_name);
            double duration = get_mp3_duration(new_path);
            time_t begin;
            time(&begin);
            time_t current_time;
            time(&current_time);
            while((double)(current_time - begin)<duration)
            {
                time(&current_time);
            }
        }
    }
    fclose(file);
}


void *normalplaythread(void *arg) {
    Logiciel *logiciel = (Logiciel *)arg;

    // Ouvrir le dossier playlist_path
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(logiciel->playlist_path)) != NULL) {
        // Lire chaque fichier dans le dossier
        while ((ent = readdir(dir)) != NULL) {
            // Ignorer les fichiers spéciaux "." et ".."
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
                continue;
            }

            // Vérifier si le fichier est un fichier MP3
            char *dot = strrchr(ent->d_name, '.');
            if (dot && strcmp(dot, ".mp3") == 0) {
                // Construire le chemin complet du fichier MP3
                char filepath[256];
                snprintf(filepath, sizeof(filepath), "%s/%s", logiciel->playlist_path, ent->d_name);

                // Supprimer l'extension .mp3 pour le nom du morceau
                *dot = '\0';
                char *track_name = ent->d_name;

                // Jouer la chanson
                playsong(logiciel->playlist_path, track_name, logiciel);

                // Ajouter l'extension .mp3 pour obtenir la durée du morceau
                snprintf(filepath, sizeof(filepath), "%s/%s.mp3", logiciel->playlist_path, track_name);
                double duration = get_mp3_duration(filepath);

                // Attendre la durée du morceau
                time_t begin, current_time;
                time(&begin);
                do {
                    time(&current_time);
                } while ((double)(current_time - begin) < duration);
            }
        }
        closedir(dir);
    } else {
        // Erreur d'ouverture du dossier
        perror("Could not open directory");
    }

    return NULL;
}

void sortedplay(GtkButton *button, gpointer user_data)
{
    Logiciel *logiciel = user_data;
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, sortedplaythread, logiciel);
    pthread_detach(thread_id);
}


void normalplay(GtkButton *button, gpointer user_data)
{
    Logiciel *logiciel = user_data;
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, normalplaythread, logiciel);
    pthread_detach(thread_id);
}



int main(int argc, char *argv[])
{
    GtkWidget *window;
    GtkBuilder *builder;

    Logiciel logiciel;

    logiciel.test=0;

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
    logiciel.playlist_window = GTK_WIDGET(gtk_builder_get_object(logiciel.builder, "playlist_window"));
    if (!logiciel.playlist_window) {
        g_warning("Failed to get playlist_window from builder");
        return 1;
    }

    FILE *file = fopen("src/username", "r");
    if (file != NULL) {
        static gchar line[256];
        if (fgets(line, sizeof(line), file) != NULL) {
            line[strcspn(line, "\n")] = '\0';
            const gchar *username = line;
            GtkLabel *label = GTK_LABEL(gtk_builder_get_object(builder, "current_username"));
            gtk_label_set_text(label, username);
        }
        fclose(file);
    } else {
        g_warning("Could not open file: username");
    }

    GtkLabel *label2 = GTK_LABEL(gtk_builder_get_object(logiciel.builder, "waiting-label"));
    logiciel.label2 =label2;



    GtkButton* create_button = GTK_BUTTON(gtk_builder_get_object(builder, "create_playlist"));
    g_signal_connect(create_button, "clicked", G_CALLBACK(create_panel_display), &logiciel);

    GtkButton* delete_playlist = GTK_BUTTON(gtk_builder_get_object(builder, "delete_playlist"));
    g_signal_connect(delete_playlist, "clicked", G_CALLBACK(delete_playlist_callback), &logiciel);

    GtkButton* normal_play = GTK_BUTTON(gtk_builder_get_object(logiciel.builder, "normal_play"));
    g_signal_connect(normal_play, "clicked", G_CALLBACK(normalplay), &logiciel);

    g_signal_connect(logiciel.playlist_window, "delete-event", G_CALLBACK(on_window_delete_event), &logiciel);

    GtkButton* ok_name = GTK_BUTTON(gtk_builder_get_object(builder, "ok_name"));
    g_signal_connect(ok_name, "clicked", G_CALLBACK(ok_name_callback), &logiciel);

    GtkTreeView *treeview = GTK_TREE_VIEW(gtk_builder_get_object(builder, "playlist_treeview"));

    system("curl localhost:8000/hit");

    // Connecter les signaux à chaque fois que create_panel est affiché
    GtkButton *browse_btn = GTK_BUTTON(gtk_builder_get_object(logiciel.builder, "browse_button"));
    g_signal_connect(browse_btn, "clicked", G_CALLBACK(browse_button_callback), &logiciel);
 
    GtkButton *sortedplaybtn = GTK_BUTTON(gtk_builder_get_object(logiciel.builder, "sortedplay"));
    g_signal_connect(sortedplaybtn, "clicked", G_CALLBACK(sortedplay), &logiciel);

    GtkButton *create_create_btn = GTK_BUTTON(gtk_builder_get_object(logiciel.builder, "create_create"));
    g_signal_connect(create_create_btn, "clicked", G_CALLBACK(create_playlist), &logiciel);
    g_signal_connect(logiciel.create_panel, "delete-event", G_CALLBACK(on_window_delete_event), &logiciel);

    GtkListStore *liststore = gtk_list_store_new(1, G_TYPE_STRING);
    gtk_tree_view_set_model(treeview, GTK_TREE_MODEL(liststore));

    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("src/playlists", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(treeview, column);

    logiciel.list = liststore;
    load_playlists("src/playlists", liststore);

    g_signal_connect(treeview, "row-activated", G_CALLBACK(on_playlist_treeview_row_activated), &logiciel);

    GtkTreeView *treeview2 = GTK_TREE_VIEW(gtk_builder_get_object(builder, "inside_playlist")); 
    GtkListStore *liststore2 = gtk_list_store_new(1, G_TYPE_STRING);
    gtk_tree_view_set_model(treeview2, GTK_TREE_MODEL(liststore2));
    GtkCellRenderer *renderer2 = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column2 = gtk_tree_view_column_new_with_attributes("Tracks", renderer2, "text", 0, NULL);
    gtk_tree_view_append_column(treeview2, column2);
    g_signal_connect(treeview2, "row-activated", G_CALLBACK(on_tracks_treeview_row_activated), &logiciel);

    logiciel.tracks = liststore2;

    g_signal_connect(window, "destroy", G_CALLBACK(on_window_close), NULL);
    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}