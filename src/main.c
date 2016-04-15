//
// Created by Kelvin on 2016/4/12.
//

#if defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#else

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

#endif

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "model.h"

#define SCREEN_FPS 60
#define BLOCK_SIZE 32
#define DEFAULT_MAP_WIDTH 9
#define DEFAULT_MAP_HEIGHT 9
#define DEFAULT_MAP_MINES 10
#define NUMBER_TEXTURES 8

typedef struct {
    SDL_Texture *texture;
    int width, height;
} TextTexture;

const Uint32 SCREEN_TICK_PER_FRAME = 1000 / SCREEN_FPS;

Map *map = NULL;

SDL_Window *gWindow = NULL;
SDL_Renderer *gRenderer = NULL;
TTF_Font *gFont = NULL;
TextTexture *numTextures[NUMBER_TEXTURES] = {0};
SDL_Texture *mineTexture = NULL;
SDL_Texture *flagTexture = NULL;
SDL_Texture *questionTexture = NULL;
SDL_Texture *explosionTexture = NULL;
SDL_Texture *errorTexture = NULL;
Mix_Chunk *clickSound = NULL;
Mix_Chunk *explosionSound = NULL;
Mix_Chunk *winSound = NULL;

SDL_Color bgColor = {0xFF, 0xFF, 0xFF, 0xFF};
SDL_Color lineColor = {0xEF, 0xEF, 0xEF, 0xFF};
SDL_Color blockColor = {0x3F, 0x7F, 0xCF, 0xFF};
SDL_Color numColors[NUMBER_TEXTURES] = {
        {0x20, 0x20, 0xFF, 0xFF},
        {0x20, 0xFF, 0x20, 0xFF},
        {0xFF, 0x20, 0x20, 0xFF},
        {0x00, 0x00, 0x8B, 0xFF},
        {0xA5, 0x2A, 0x2A, 0xFF},
        {0x00, 0x64, 0x00, 0xFF},
        {0x00, 0x00, 0x00, 0xFF},
        {0xA9, 0xA9, 0xA9, 0xFF}
};

static bool init(int argc, char **argv);

static bool loadMedia();

static SDL_Texture *loadImageTexture(const char *path);

static TextTexture *loadTextTexture(TTF_Font *font, const char *str, SDL_Color color);

static Mix_Chunk *loadSound(const char *path);

static void close();


static bool init(int argc, char **argv) {
    bool success = true;

    int width = DEFAULT_MAP_WIDTH, height = DEFAULT_MAP_HEIGHT, mines = DEFAULT_MAP_MINES;
    if (argc > 3) {
        width = (int) strtol(argv[1], NULL, 10);
        height = (int) strtol(argv[2], NULL, 10);
        mines = (int) strtol(argv[3], NULL, 10);
    }
    int screenWidth = width * BLOCK_SIZE;
    int screenHeight = height * BLOCK_SIZE;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("SDL could not initialize! %s\n", SDL_GetError());
        success = false;
    }
    else {
        if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
            printf("Warning: Linear texture filtering not enabled!");
        }
        gWindow = SDL_CreateWindow("Mine Sweeper", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth,
                                   screenHeight, SDL_WINDOW_SHOWN);
        if (gWindow == NULL) {
            printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
            success = false;
        }
        else {
            gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
            if (gRenderer == NULL) {
                printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
                success = false;
            }
            else {
                SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
                int imgFlags = IMG_INIT_PNG;
                if (!(IMG_Init(imgFlags) & imgFlags)) {
                    printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
                    success = false;
                } else {
                    SDL_Surface *iconSurface = IMG_Load("res/mine.png");
                    if (iconSurface == NULL) {
                        printf("Unable to load icon image! SDL_image Error: %s\n", IMG_GetError());
                    } else {
                        SDL_SetWindowIcon(gWindow, iconSurface);
                        SDL_FreeSurface(iconSurface);
                    }
                }
                if (TTF_Init() == -1) {
                    printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
                    success = false;
                }
                if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
                    printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
                    success = false;
                }
            }
        }
    }

    map = createMap(width, height, mines);
    if (!map)
        success = false;
    else {
#ifdef debug
        printMap(map);
#endif
    }
    return success;
}

static bool loadMedia() {
    bool success = true;
    gFont = TTF_OpenFont("res/DroidSans.ttf", (int) (BLOCK_SIZE * 0.75));
    if (gFont == NULL) {
        printf("Failed to load lazy font! SDL_ttf Error: %s\n", TTF_GetError());
        success = false;
    } else {
        char numStr[2];
        for (int i = 0; i < NUMBER_TEXTURES; ++i) {
            snprintf(numStr, 2, "%d", i + 1);
            TextTexture *texture = loadTextTexture(gFont, numStr, numColors[i]);
            if (texture == NULL) {
                success = false;
                break;
            }
            numTextures[i] = texture;
        }
    }
    if (success) {
        mineTexture = loadImageTexture("res/mine.png");
        flagTexture = loadImageTexture("res/flag.png");
        questionTexture = loadImageTexture("res/question.png");
        explosionTexture = loadImageTexture("res/explosion.png");
        errorTexture = loadImageTexture("res/error.png");
        if (mineTexture == NULL || flagTexture == NULL || questionTexture == NULL || explosionTexture == NULL ||
            errorTexture == NULL)
            success = false;
    }
    if (success) {
        clickSound = loadSound("res/click.wav");
        explosionSound = loadSound("res/explosion.wav");
        winSound = loadSound("res/win.wav");
        if (clickSound == NULL || explosionSound == NULL || winSound == NULL)
            success = false;
    }
    return success;
}

static Mix_Chunk *loadSound(const char *path) {
    Mix_Chunk *sound = Mix_LoadWAV(path);
    if (sound == NULL) {
        printf("Failed to load sound effect! SDL_mixer Error: %s\n", Mix_GetError());
    }
    return sound;
}

static SDL_Texture *loadImageTexture(const char *path) {
    SDL_Texture *newTexture = NULL;
    SDL_Surface *loadedSurface = IMG_Load(path);
    if (loadedSurface == NULL) {
        printf("Unable to load image %s! SDL_image Error: %s\n", path, IMG_GetError());
    }
    else {
        newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
        if (newTexture == NULL) {
            printf("Unable to create texture from %s! SDL Error: %s\n", path, SDL_GetError());
        }
        SDL_FreeSurface(loadedSurface);
    }
    return newTexture;
}

static TextTexture *loadTextTexture(TTF_Font *font, const char *str, SDL_Color color) {
    SDL_Surface *textSurface = TTF_RenderText_Blended(font, str, color);
    if (textSurface != NULL) {
        TextTexture *text = NULL;
        SDL_Texture *texture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
        if (texture == NULL) {
            printf("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
        } else {
            text = (TextTexture *) malloc(sizeof(TextTexture));
            text->texture = texture;
            text->width = textSurface->w;
            text->height = textSurface->h;
        }
        SDL_FreeSurface(textSurface);
        return text;
    } else {
        printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
    }
    return NULL;
}

static void destroyTextTexture(TextTexture *texture) {
    SDL_DestroyTexture(texture->texture);
    free(texture);
}


static void close() {
    if (map != NULL) {
        destroyMap(map);
        map = NULL;
    }

    for (int i = 0; i < NUMBER_TEXTURES; ++i) {
        TextTexture *texture = numTextures[i];
        if (texture != NULL)
            destroyTextTexture(texture);
        numTextures[i] = NULL;
    }
    if (mineTexture != NULL) {
        SDL_DestroyTexture(mineTexture);
        mineTexture = NULL;
    }
    if (flagTexture != NULL) {
        SDL_DestroyTexture(flagTexture);
        flagTexture = NULL;
    }
    if (questionTexture != NULL) {
        SDL_DestroyTexture(questionTexture);
        questionTexture = NULL;
    }
    if (explosionTexture != NULL) {
        SDL_DestroyTexture(explosionTexture);
        explosionTexture = NULL;
    }
    if (errorTexture != NULL) {
        SDL_DestroyTexture(errorTexture);
        errorTexture = NULL;
    }
    if (clickSound != NULL) {
        Mix_FreeChunk(clickSound);
        clickSound = NULL;
    }
    if (explosionSound != NULL) {
        Mix_FreeChunk(explosionSound);
        explosionSound = NULL;
    }
    if (winSound != NULL) {
        Mix_FreeChunk(winSound);
        winSound = NULL;
    }

    TTF_CloseFont(gFont);
    gFont = NULL;

    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    gWindow = NULL;
    gRenderer = NULL;

    TTF_Quit();
    Mix_Quit();
    IMG_Quit();
    SDL_Quit();
}

int main(int argc, char **argv) {
    if (!init(argc, argv)) {
        printf("Failed to initialize!\n");
    }
    else {
        if (!loadMedia()) {
            printf("Failed to load media!\n");
        }
        else {
            bool quit = false;
            bool game_over = false;
            SDL_Event e;
            while (!quit) {
                Uint32 capStart = SDL_GetTicks();
                while (SDL_PollEvent(&e) != 0) {
                    if (e.type == SDL_QUIT) {
                        quit = true;
                    } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                        if (!game_over) {
                            int button = e.button.button;
                            int column = e.button.x / BLOCK_SIZE;
                            int row = e.button.y / BLOCK_SIZE;
                            if (button == SDL_BUTTON_LEFT) {
                                int lastRemainBlocks = map->remainBlocks;
                                if (e.button.clicks == 1) {
                                    if (visitMap(map, row, column) == -1) {
                                        game_over = true;
                                        Mix_PlayChannel(-1, explosionSound, 0);
                                    }
                                } else if (e.button.clicks == 2) {
                                    if (quickVisitMap(map, row, column) == -1) {
                                        game_over = true;
                                        Mix_PlayChannel(-1, explosionSound, 0);
                                    }
                                }
                                if (map->remainBlocks < lastRemainBlocks) {
                                    Mix_PlayChannel(-1, clickSound, 0);
                                }
                                if (map->remainBlocks == 0) {
                                    game_over = true;
                                    Mix_PlayChannel(-1, winSound, 0);
                                }
                            } else if (button == SDL_BUTTON_RIGHT) {
                                markMap(map, e.button.y / BLOCK_SIZE, e.button.x / BLOCK_SIZE);
                            }
                        }
                    }
                }
                SDL_SetRenderDrawColor(gRenderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
                SDL_RenderClear(gRenderer);

                SDL_SetRenderDrawColor(gRenderer, lineColor.r, lineColor.g, lineColor.b, lineColor.a);
                SDL_Rect windowRect = {0, 0, BLOCK_SIZE * map->width, BLOCK_SIZE * map->height};
                SDL_RenderDrawRect(gRenderer, &windowRect);
                for (int i = 1; i < map->height; ++i)
                    SDL_RenderDrawLine(gRenderer, 0, i * BLOCK_SIZE, map->width * BLOCK_SIZE, i * BLOCK_SIZE);
                for (int i = 1; i < map->width; ++i)
                    SDL_RenderDrawLine(gRenderer, i * BLOCK_SIZE, 0, i * BLOCK_SIZE, map->height * BLOCK_SIZE);

                SDL_SetRenderDrawColor(gRenderer, blockColor.r, blockColor.g, blockColor.b, blockColor.a);
                for (int i = 0; i < map->height; ++i) {
                    for (int j = 0; j < map->width; ++j) {
                        SDL_Rect iconRect = {
                                (int) ((j + 0.125) * BLOCK_SIZE),
                                (int) ((i + 0.125) * BLOCK_SIZE),
                                (int) (BLOCK_SIZE * 0.75),
                                (int) (BLOCK_SIZE * 0.75)
                        };
                        unsigned char status = map->status[i][j];
                        bool isMine = map->field[i][j];
                        if (status != MAP_STATUS_VISITED) {
                            bool wrongMarkMine = !isMine && status == MAP_STATUS_MARK_MINE;
                            if (game_over && (isMine || wrongMarkMine)) {
                                SDL_RenderCopy(gRenderer, mineTexture, NULL, &iconRect);
                                if (wrongMarkMine) {
                                    SDL_RenderCopy(gRenderer, errorTexture, NULL, &iconRect);
                                }
                            } else {
                                SDL_Rect rect = {
                                        j * BLOCK_SIZE + 1,
                                        i * BLOCK_SIZE + 1,
                                        BLOCK_SIZE - 2,
                                        BLOCK_SIZE - 2
                                };
                                SDL_RenderFillRect(gRenderer, &rect);
                            }
                        } else {
                            if (map->field[i][j]) {
                                SDL_RenderCopy(gRenderer, mineTexture, NULL, &iconRect);
                                SDL_RenderCopy(gRenderer, explosionTexture, NULL, &iconRect);
                            } else {
                                unsigned char indicator = map->indicator[i][j];
                                if (indicator > 0) {
                                    TextTexture *num = numTextures[indicator - 1];
                                    SDL_Rect rect = {
                                            j * BLOCK_SIZE + (BLOCK_SIZE - num->width) / 2,
                                            i * BLOCK_SIZE + (BLOCK_SIZE - num->height) / 2,
                                            num->width,
                                            num->height
                                    };
                                    SDL_RenderCopy(gRenderer, num->texture, NULL, &rect);
                                }
                            }
                        }
                        if (!game_over) {
                            if (status == MAP_STATUS_MARK_MINE) {
                                SDL_RenderCopy(gRenderer, flagTexture, NULL, &iconRect);
                            } else if (status == MAP_STATUS_MARK_NOT_DETERMINED) {
                                SDL_RenderCopy(gRenderer, questionTexture, NULL, &iconRect);
                            }
                        }
                    }
                }

                SDL_RenderPresent(gRenderer);
                Uint32 frameTicks = SDL_GetTicks() - capStart;
                if (frameTicks < SCREEN_TICK_PER_FRAME) {
                    SDL_Delay(SCREEN_TICK_PER_FRAME - frameTicks);
                }
            }
        }
    }
    close();
    return EXIT_SUCCESS;
}