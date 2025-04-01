///
/// COMPILARE con : clang SOURCEFILE -lSDL3 -lSDL3_image -lGL -Wall -pedantic -std=c23 -g
///

#include <SDL3/SDL_init.h>
#define SDL_MAIN_USE_CALLBACKS 1 /// usa le callback al posto di main()
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>

#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

///
/// Variabili Globali per la finestra
///
typedef struct
{
    float red;
    float green;
    float blue;
    float alpha;
} Color;

///
/// State context of the App
///
typedef struct Appstate
{
    Uint32 WINDOW_FLAGS;
    int WIN_WIDTH;
    int WIN_HEIGHT;
    int NEW_WIN_WIDTH;
    int NEW_WIN_HEIGHT;
    const char *TITOLO_FINESTRA;
    const char *VERSION;
    Color color;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    char *file_list;
    int MAX_CHARS;
    SDL_FRect dest_rect;
    bool running;
    DIR *directory;
    char *path;

    size_t total_files;
    size_t current_idx;
    size_t file_idx;
    char *file_head;

    bool is_Fullscreen;

} Appstate;

///
/// call to read the current dir content
///
void load_dir(void *appstate);

///
/// INIZIALIZZAZIONE
///
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    puts(argv[0]);
    if(argc < 2 || argv[1] == NULL)
    {
        SDL_Log("./img_viewer3 DIR_PATH");
        return SDL_APP_SUCCESS;
    }
    ///
    /// Imposto l'indirizzo del puntatore a puntatore appstate passato dal parametro
    /// all'indirizzo del puntatore a struct utente Appstate state creata da me
    Appstate *state = calloc(1, sizeof(Appstate));
    *appstate = state;
    ///
    /// Setup the App state/context
    ///
    state->WINDOW_FLAGS = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
    state->WIN_WIDTH = 800;
    state->WIN_HEIGHT = 600;
    state->NEW_WIN_WIDTH = 800;
    state->NEW_WIN_HEIGHT = 600;
    state->TITOLO_FINESTRA = "SDL3 Image Viewer";

    state->VERSION = "0.1";

    state->color.red = 0.2f;
    state->color.green = 0.2f;
    state->color.blue = 0.2f;
    state->color.alpha = 1.0f;

    state->running = true;

    state->MAX_CHARS = 256;
    strcpy(state->path, argv[1]);
    state->directory = NULL;
    state->total_files = 0;
    state->current_idx = 0;
    state->file_idx = 0;
    state->file_head = NULL;

    state->dest_rect.h = 0;
    state->dest_rect.w = 0;
    state->dest_rect.x = 0;
    state->dest_rect.y = 0;

    state->is_Fullscreen = false;

    ///
    /// Imposto i METADATA della app
    ///
    SDL_SetAppMetadata(state->TITOLO_FINESTRA, state->VERSION, NULL);

    ///
    /// Inizializzo SDL
    ///
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("Impossibile inizializzare SDL3: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    ///
    /// Creo Finestra tipo OpenGL
    ///
    state->window = SDL_CreateWindow(state->TITOLO_FINESTRA, state->WIN_WIDTH, state->WIN_HEIGHT, state->WINDOW_FLAGS);
    if (state->window == NULL)
    {
        SDL_Log("Impossibile Creare finestra : %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    ///
    /// Creo Renderer
    ///
    state->renderer = SDL_CreateRenderer(state->window, NULL);
    if (state->renderer == NULL)
    {
        SDL_Log("Impossibile creare renderer : %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    ///
    /// Load list of files in current dir
    ///
    load_dir(state);

    SDL_GL_SetSwapInterval(1);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    Appstate *state = appstate;
    ///
    /// Esci su pulsante di chiusura o ESC
    ///
    if (event->type == SDL_EVENT_QUIT || event->key.key == SDLK_ESCAPE)
    {
        state->running = false;
        return SDL_APP_SUCCESS;
    }

    switch (event->key.key)
    {
    case SDLK_F11:
        SDL_SetWindowFullscreen(state->window, true);
        break;
    case SDLK_F12:
        SDL_SetWindowFullscreen(state->window, false);
        break;
    case SDLK_RIGHT:
        if (state->file_idx > state->total_files - 1)
        {
            state->file_idx = 0;
            state->file_list = state->file_head;
            SDL_Log("FILE_IDX : %5zu\tFILE : %s\n ", state->file_idx, state->file_list);
        }
        else
        {
            state->file_idx++;
            state->file_list += state->MAX_CHARS;
            printf("FILE_IDX : %5zu\tFILE : %s\n ", state->file_idx, state->file_list);
        }

        SDL_Log("Premuto tasto l");
        break;
    case SDLK_LEFT:
        if (state->file_idx == 0)
        {
            state->file_list = state->file_head + (state->total_files * (size_t)state->MAX_CHARS) - (size_t)state->MAX_CHARS;
            state->file_idx = state->total_files - 1;
            SDL_Log("FILE_IDX : %5zuFILE : %s\n ", state->file_idx, state->file_list);
        }
        else
        {
            state->file_idx--;
            state->file_list -= state->MAX_CHARS;
            printf("FILE_IDX : %5zuFILE : %s\n ", state->file_idx, state->file_list);
        }
        SDL_Log("Premuto tasto h");
        break;
    default:
        break;
    }
    switch (event->type)
    {
    case SDL_EVENT_WINDOW_RESIZED:
        SDL_GetWindowSizeInPixels(state->window, &state->NEW_WIN_WIDTH, &state->NEW_WIN_HEIGHT);
        SDL_Log("WIndow resized , new w : %5d\tnew h : %5d", state->NEW_WIN_WIDTH, state->NEW_WIN_HEIGHT);
        state->WIN_WIDTH = state->NEW_WIN_WIDTH;
        state->WIN_HEIGHT = state->NEW_WIN_HEIGHT;
        break;
    default:
        break;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    ///
    /// Copiamo il puntatore a puntatore in una variabile locale
    ///
    Appstate *state = appstate;

    ///
    /// SETUP TEXTURE
    ///

    // TEXTURE
    state->texture = IMG_LoadTexture(state->renderer, state->file_list);
    if (state->texture == NULL)
    {
        SDL_Log("Impossibile creare Texture : %s", SDL_GetError());
        return SDL_APP_CONTINUE;
    }

    // SRC RECT

    // ratio = w / h
    float src_ratio = (float)state->texture->w / (float)state->texture->h;
    SDL_Log("texture_width : %5d\ttexture_height : %5d", state->texture->w, state->texture->h);
    SDL_Log("src_ratio : %2.f", src_ratio);

    // DEST RECT
    // LEGGO SIZE ATTUALE DELLA WINDOW
    SDL_GetWindowSize(state->window, &state->WIN_WIDTH, &state->WIN_HEIGHT);
    SDL_Log("WINDOW SIZE w : %5d\th : %5d", state->WIN_WIDTH, state->WIN_HEIGHT);
    // IMPOSTO WIDTH DEL RENDERING IMMAGINE A WIN WIDTH
    float dest_width = (float)state->WIN_WIDTH;
    // CALCOLO DEL RENDERING IMAGINE HEIGHT = WIDTH / RATIO
    float dest_height = (float)state->WIN_WIDTH / src_ratio;
    // in questo caso regolo y_offset
    float y_offset = (float)state->WIN_HEIGHT / 2.0f - dest_height / 2.0f;
    float x_offset = 0;
    // CHECK SE DEST HEIGHT > WIN HEIGHT
    if (dest_height > state->WIN_HEIGHT)
    {
        dest_height = (float)state->WIN_HEIGHT;
        dest_width = (float)state->WIN_HEIGHT * src_ratio;
        x_offset = (float)state->WIN_WIDTH / 2.0f - dest_width / 2.0f;
        y_offset = 0;
    }
    state->dest_rect = (SDL_FRect){.w = dest_width, .h = dest_height, .x = x_offset, .y = y_offset};
    SDL_Log("dest_rec w : %6.1f\th : %6.1f\tx_offset : %6.1fd\ty_offset : %6.1f ", dest_width, dest_height, x_offset,
            y_offset);

    SDL_SetRenderDrawColorFloat(state->renderer, state->color.red, state->color.green, state->color.blue,
                                state->color.alpha);
    SDL_RenderClear(state->renderer);
    SDL_RenderTexture(state->renderer, state->texture, NULL, &state->dest_rect);

    SDL_RenderPresent(state->renderer);

    return SDL_APP_CONTINUE; /* carry on with the program! */
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    ///
    /// Copiamo il puntatore a puntatore in una variabile locale
    ///
    Appstate *state = appstate;

    ///
    /// Liberiamo le risorse allocate
    ///
    // CLEANUP AND FINISH
    if (state->texture != NULL)
    {
        SDL_DestroyTexture(state->texture);
    }
    SDL_DestroyRenderer(state->renderer);
    SDL_DestroyWindow(state->window);
    free(state->file_list);
    free(state);
    if (result != SDL_APP_SUCCESS)
    {
        SDL_Log("Errore durante l'esecuzione");
    }
}

void load_dir(void *appstate)
{
    Appstate *state = appstate;

    // APRO DIRECTORY
    state->directory = opendir(state->path);
    // ESCO SE IMPOSSIBILE APRIRE DIR
    if (state->directory == NULL)
    {
        SDL_Log("Impossibile accedere a \".\"");
        SDL_AppQuit(state, SDL_APP_SUCCESS);
    }
    // LEGGO SIZE DEI FILES IN DIR
    while (readdir(state->directory))
    {
        state->total_files++;
    }
    closedir(state->directory);
    state->directory = opendir(state->path);
    printf("Trovati %zu files\n", state->total_files);
    // PREPARO L'ARRAY DI 256 CHAR per file
    state->file_list = calloc(state->total_files, (size_t)state->MAX_CHARS);
    state->file_head = state->file_list;
    // RIEMPIO LA LISTA DEI FILES IN DIR
    // struct dirent *entry;
    // SALVO INIZIO LISTA
    if (state->file_list == NULL)
    {
        SDL_Log("ERRORE ALLOCAZIONE MEMORIA\n");
        SDL_AppQuit(state, SDL_APP_FAILURE);
    }
    // salvo testa della lista
    char *temp_file_list = state->file_list;
    // RIEMPIO LA LISTA
    for (size_t i = 0; i < state->total_files; i++)
    {
        strcpy(state->file_list, readdir(state->directory)->d_name);
        // strlcpy(file_list, dir_item,strlen(file_list));
        puts(state->file_list);
        state->file_list += state->MAX_CHARS;
    }
    // ripristino testa della lista
    state->file_list = temp_file_list;
    // salvo testa della lista
    temp_file_list = state->file_list;
    // STAMPO LISTA FILE TROVATI
    for (size_t i = 0; i < state->total_files; i++)
    {
        printf("IDX : %10zu\tfile : %s\n", i, state->file_list);
        state->file_list += state->MAX_CHARS;
    }
    // ripristino testa della lista
    state->file_list = temp_file_list;
    closedir(state->directory);
}
