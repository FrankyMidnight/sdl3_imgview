#define SDL_MAIN_NOIMPL
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// COMPILARE CON : clang -g -Wall -Wextra -pedantic -std=c23 image_viewer3.c -lSDL3 -lSDL3_image
int WIN_WIDTH = 800;
int WIN_HEIGHT = 600;
const Uint32 WINDOW_FLAGS = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
SDL_Window *window = NULL;
bool is_fullscreen = false;
SDL_Surface *surface = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *texture = NULL;
SDL_FRect dest_rect = {};
bool running = true;
const char *path = ".";
int Init(void);
void Quit(void);
void Draw(void);
void Update(void);
void update_windowsize(void);
void load_dir(void);
#define MAX_CHARS 256
DIR *directory = NULL;
char *file_list;
size_t total_files = {0};
size_t current_idx = 0;
size_t file_idx = 0;
char *file_head;

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Launch with : image_viewer [DIR]\n");
        return 0;
    }
    if (argv[1])
    {
        path = argv[1];
    }

    Init();
    load_dir();
    const Uint32 FPS = 60;                  // FPS desiderati
    const Uint32 target_delay = 1000 / FPS; // target delay = 16 ms per 60 FPS

    Uint32 frame_start; // ticks a inizio loop
    Uint32 delta_time;   // tempo trascorso
    while (running)
    {
        frame_start = (Uint32)SDL_GetTicks(); // leggo tick da inizializzazione SDL
        Draw();
        Update();
        delta_time = (Uint32)SDL_GetTicks() - frame_start; // calcolo delta time = end – beg

        if (target_delay > delta_time) // se target delay è maggiore di delta time
        {
            SDL_Delay(target_delay - delta_time); // delay =  target_delay – delta time
        }
    }
    Quit();
    return 0;
}

int Init()
{
    // SDL INIT
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("Impossibile inizializzare SDL3 : %s", SDL_GetError());
        return 1;
    }

    // WINDOW
    window = SDL_CreateWindow("test", WIN_WIDTH, WIN_HEIGHT, WINDOW_FLAGS);
    if (window == NULL)
    {
        SDL_Log("Impossibile creare finestra : %s", SDL_GetError());
        return 2;
    }

    // RENDERER
    renderer = SDL_CreateRenderer(window, NULL);
    if (renderer == NULL)
    {
        SDL_Log("Errore di inizializzazione del renderer : %s", SDL_GetError());
        return 4;
    }
    return 0;
}
void Quit()
{
    // CLEANUP AND FINISH
    if (texture != NULL)
    {
        SDL_DestroyTexture(texture);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    free(file_list);
}
void Draw()
{
    
    // TEXTURE
    texture = IMG_LoadTexture(renderer, file_list);
    if (texture == NULL)
    {
        // SDL_Log("Impossibile creare Texture : %s", SDL_GetError());
        return;
    }

    // SRC RECT

    // ratio = w / h
    float src_ratio = (float)texture->w / (float)texture->h;
    // SDL_Log("texture_width : %5d\ttexture_height : %5d", texture->w, texture->h);
    // SDL_Log("src_ratio : %2.f", src_ratio);

    // DEST RECT
    // LEGGO SIZE ATTUALE DELLA WINDOW
    SDL_GetWindowSize(window, &WIN_WIDTH, &WIN_HEIGHT);
    // SDL_Log("WINDOW SIZE w : %5d\th : %5d", WIN_WIDTH, WIN_HEIGHT);
    // IMPOSTO WIDTH DEL RENDERING IMMAGINE A WIN WIDTH
    float dest_width = (float)WIN_WIDTH;
    // CALCOLO DEL RENDERING IMAGINE HEIGHT = WIDTH / RATIO
    float dest_height = (float)WIN_WIDTH / src_ratio;
    // in questo caso regolo y_offset
    float y_offset = (float)WIN_HEIGHT / 2.0f - dest_height / 2.0f;
    float x_offset = 0;
    // CHECK SE DEST HEIGHT > WIN HEIGHT
    if (dest_height > WIN_HEIGHT)
    {
        dest_height = (float)WIN_HEIGHT;
        dest_width = (float)WIN_HEIGHT * src_ratio;
        x_offset = (float)WIN_WIDTH / 2.0f - dest_width / 2.0f;
        y_offset = 0;
    }
    dest_rect = (SDL_FRect){.w = dest_width, .h = dest_height, .x = x_offset, .y = y_offset};
    // SDL_Log("dest_rec w : %6.1f\th : %6.1f\tx_offset : %6.1fd\ty_offset : %6.1f ", dest_width, dest_height, x_offset,
    //         y_offset);
    // DELAY A LITTLE
    // usleep(100);
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, texture, NULL, &dest_rect);
    SDL_RenderPresent(renderer);
}
void Update()
{
    // LOOP
    SDL_Event event;
    // SVUOTA QUEUE EVENTI
    while (SDL_PollEvent(&event))
    {
        // ESCI SU CLICK CHIUSURA FINESTRA
        if (event.type == SDL_EVENT_QUIT)
        {
            running = false;
        }
        if (event.window.type == SDL_EVENT_WINDOW_RESIZED)
        {
            update_windowsize();
        }
        if(event.type == SDL_EVENT_KEY_DOWN)
        {

            switch (event.key.scancode)
            {
                // ESCI SU PRESSIONE TASTO ESCAPE E TASTO Q
            case SDL_SCANCODE_ESCAPE:
            case SDL_SCANCODE_Q:
                running = false;
                break;
                // FULLSCREEN
            case SDL_SCANCODE_F11:
                if (is_fullscreen == true)
                {
                    SDL_SetWindowFullscreen(window, false);
                    is_fullscreen = false;
                }
                else
                {
                    SDL_SetWindowFullscreen(window, true);
                    is_fullscreen = true;
                }
                break;
                // NEXT PIC
            case SDL_SCANCODE_RIGHT:
                if (file_idx > total_files - 1)
                {
                    file_idx = 0;
                    file_list = file_head;
                }
                else
                {
                    file_idx++;
                    file_list += MAX_CHARS;
                }
                break;
                // PREVIOUS PIC
            case SDL_SCANCODE_LEFT:
                if (file_idx == 0)
                {
                    file_list = file_head + ((int)total_files * MAX_CHARS) - MAX_CHARS;
                    file_idx = total_files - 1;
                }
                else
                {
                    file_idx--;
                    file_list -= MAX_CHARS;
                }
                break;
            default:
                break;
            }
        }
    }
}

void load_dir()
{
    // APRO DIRECTORY
    directory = opendir(path);
    // ESCO SE IMPOSSIBILE APRIRE DIR
    if (directory == NULL)
    {
        SDL_Log("Impossibile accedere a \".\"");
        Quit();
    }
    // LEGGO COUNT DEI FILES IN DIR
    while (readdir(directory))
    {
        total_files++;
    }
    // SDL_Log("total files %ld", total_files);
    // rimuovo 2 per . e ..
    total_files -= 2;
    // SDL_Log("total files %ld", total_files);
    // esco se nessun file
    if(total_files == 0)
    {
        running = false;
        SDL_Log("Niente immagini in questa directory");
        return;
    }
    closedir(directory);
    directory = opendir(path);
    SDL_Log("Trovati %zu files\n", total_files);
    // PREPARO L'ARRAY DI 256 CHAR per file
    file_list = calloc(total_files , (size_t)MAX_CHARS);
    file_head = file_list;
    // RIEMPIO LA LISTA DEI FILES IN DIR
    // struct dirent *entry;
    // SALVO INIZIO LISTA
    if (file_list == NULL)
    {
        SDL_Log("ERRORE ALLOCAZIONE MEMORIA\n");
        Quit();
    }
    // salvo testa della lista
    char *temp_file_list = file_list;
    // RIEMPIO LA LISTA
    char entry[MAX_CHARS] = {};
    for (size_t i = 0; i < total_files; i++)
    {
        strcpy(entry, readdir(directory)->d_name);
        if(strcmp(entry,"." ) == 0)
        {
            // SDL_Log("Trovato file .");
            continue;
        }
        if(strcmp(entry,".." ) == 0)
        {
            // SDL_Log("Trovato file ..");
            continue;
        }
        strcpy(file_list, entry);
        // strlcpy(file_list, dir_item,strlen(file_list));
        // SDL_Log("%s", file_list);
        file_list += MAX_CHARS;
    }
    // ripristino testa della lista
    file_list = temp_file_list;
    // salvo testa della lista
    temp_file_list = file_list;
    // STAMPO LISTA FILE TROVATI
    for (size_t i = 0; i < total_files; i++)
    {
        SDL_Log("IDX : %10zu\tfile : %s\n", i, file_list);
        file_list += MAX_CHARS;
    }
    // ripristino testa della lista
    file_list = temp_file_list;
    closedir(directory);
}

void update_windowsize()
{
    int w = 0;
    int h = 0;
    SDL_GetWindowSizeInPixels(window, &w, &h);
    SDL_Log("Window resized , new w : %5d\tnew h : %5d", w, h);
    WIN_WIDTH = w;
    WIN_HEIGHT = h;
    Draw();
}
