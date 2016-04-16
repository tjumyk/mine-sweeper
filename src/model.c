//
// Created by Kelvin on 2016/4/12.
//

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "model.h"

static void initMap(Map *map);

static void finaliseMap(Map *map, int startRow, int startColumn);

Map *createMap(int width, int height, int mines) {
    if (width <= 0 || height <= 0 || width * height <= mines)
        return NULL;
    Map *map = (Map *) malloc(sizeof(Map));
    map->width = width;
    map->height = height;
    map->totalMines = map->remainMines = mines;
    map->remainBlocks = width * height - mines;
    map->field = malloc(sizeof(bool *) * height);
    map->indicator = malloc(sizeof(unsigned char *) * height);
    map->status = malloc(sizeof(unsigned char *) * height);
    for (int i = 0; i < height; ++i) {
        map->field[i] = malloc(sizeof(bool) * width);
        map->indicator[i] = malloc(sizeof(unsigned char) * width);
        map->status[i] = malloc(sizeof(unsigned char) * width);
    }
    initMap(map);
    return map;
}

void printMap(Map *map) {
    for (int i = 0; i < map->height; ++i) {
        for (int j = 0; j < map->width; ++j) {
            if (map->field[i][j])
                putchar('X');
            else
                putchar('-');
            putchar(' ');
        }
        putchar('\n');
    }
}

void destroyMap(Map *map) {
    for (int i = 0; i < map->height; ++i) {
        free(map->field[i]);
        free(map->indicator[i]);
        free(map->status[i]);
    }
    free(map->field);
    free(map->indicator);
    free(map->status);
}

int visitMap(Map *map, int row, int column) {
    if (map->status[row][column] != MAP_STATUS_DEFAULT)
        return 0;
    if (map->firstVisit) {
        finaliseMap(map, row, column);
        map->firstVisit = false;
    }
    map->status[row][column] = MAP_STATUS_VISITED;
    if (map->field[row][column]) {
        return -1;
    } else {
        --map->remainBlocks;
        if (map->indicator[row][column] == 0) {
            if (row > 0) {
                visitMap(map, row - 1, column);
                if (column > 0)
                    visitMap(map, row - 1, column - 1);
                if (column < map->width - 1)
                    visitMap(map, row - 1, column + 1);
            }
            if (row < map->height - 1) {
                visitMap(map, row + 1, column);
                if (column > 0)
                    visitMap(map, row + 1, column - 1);
                if (column < map->width - 1)
                    visitMap(map, row + 1, column + 1);
            }
            if (column > 0)
                visitMap(map, row, column - 1);
            if (column < map->width - 1)
                visitMap(map, row, column + 1);
        }
    }
    return 0;
}

int quickVisitMap(Map *map, int row, int column) {
    if (map->status[row][column] != MAP_STATUS_VISITED)
        return visitMap(map, row, column);
    int indicator = map->indicator[row][column];
    if (row > 0) {
        if (map->status[row - 1][column] == MAP_STATUS_MARK_MINE)
            --indicator;
        if (column > 0 && map->status[row - 1][column - 1] == MAP_STATUS_MARK_MINE)
            --indicator;
        if (column < map->width - 1 && map->status[row - 1][column + 1] == MAP_STATUS_MARK_MINE)
            --indicator;
    }
    if (row < map->height - 1) {
        if (map->status[row + 1][column] == MAP_STATUS_MARK_MINE)
            --indicator;
        if (column > 0 && map->status[row + 1][column - 1] == MAP_STATUS_MARK_MINE)
            --indicator;
        if (column < map->width - 1 && map->status[row + 1][column + 1] == MAP_STATUS_MARK_MINE)
            --indicator;
    }
    if (column > 0 && map->status[row][column - 1] == MAP_STATUS_MARK_MINE)
        --indicator;
    if (column < map->width - 1 && map->status[row][column + 1] == MAP_STATUS_MARK_MINE)
        --indicator;

    if (indicator <= 0) {
        if (row > 0) {
            if (visitMap(map, row - 1, column) == -1)
                return -1;
            if (column > 0 && visitMap(map, row - 1, column - 1) == -1)
                return -1;
            if (column < map->width - 1 && visitMap(map, row - 1, column + 1) == -1)
                return -1;
        }
        if (row < map->height - 1) {
            if (visitMap(map, row + 1, column) == -1)
                return -1;
            if (column > 0 && visitMap(map, row + 1, column - 1) == -1)
                return -1;
            if (column < map->width - 1 && visitMap(map, row + 1, column + 1) == -1)
                return -1;
        }
        if (column > 0 && visitMap(map, row, column - 1) == -1)
            return -1;
        if (column < map->width - 1 && visitMap(map, row, column + 1) == -1)
            return -1;
    }
    return 0;
}

void markMap(Map *map, int row, int column) {
    switch (map->status[row][column]) {
        case MAP_STATUS_DEFAULT:
            map->status[row][column] = MAP_STATUS_MARK_MINE;
            --map->remainMines;
            break;
        case MAP_STATUS_MARK_MINE:
            map->status[row][column] = MAP_STATUS_MARK_NOT_DETERMINED;
            ++map->remainMines;
            break;
        case MAP_STATUS_MARK_NOT_DETERMINED:
            map->status[row][column] = MAP_STATUS_DEFAULT;
            break;
        default:
            break;
    }
}


static void initMap(Map *map) {
    map->firstVisit = true;
    for (int i = 0; i < map->height; ++i) {
        for (int j = 0; j < map->width; ++j) {
            map->field[i][j] = false;
            map->indicator[i][j] = 0;
            map->status[i][j] = MAP_STATUS_DEFAULT;
        }
    }
    srand((unsigned int) time(NULL));
    int total = map->width * map->height, selected = 0, current = 0;
    while (current < total && selected < map->totalMines) {
        int range = total - current;
        int pick = map->totalMines - selected;
        if (rand() % range < pick) {
            ++selected;
            int row = current / map->width, column = current % map->width;
            map->field[row][column] = true;
        }
        ++current;
    }
}

static void finaliseMap(Map *map, int startRow, int startColumn) {
    if (map->field[startRow][startColumn]) {
        bool replaced = false;
        for (int i = 0; i < map->height; ++i) {
            for (int j = 0; j < map->width; ++j) {
                if (!map->field[i][j]) {
                    map->field[i][j] = true;
                    replaced = true;
                    break;
                }
            }
            if (replaced)
                break;
        }
        map->field[startRow][startColumn] = false;
#ifdef debug
        puts("Adjusted map to avoid first-click explosion");
        printMap(map);
#endif
    }
    for (int i = 0; i < map->height; ++i) {
        for (int j = 0; j < map->width; ++j) {
            if (map->field[i][j]) {
                if (i > 0) {
                    ++map->indicator[i - 1][j];
                    if (j > 0)
                        ++map->indicator[i - 1][j - 1];
                    if (j < map->width - 1)
                        ++map->indicator[i - 1][j + 1];
                }
                if (i < map->height - 1) {
                    ++map->indicator[i + 1][j];
                    if (j > 0)
                        ++map->indicator[i + 1][j - 1];
                    if (j < map->width - 1)
                        ++map->indicator[i + 1][j + 1];
                }
                if (j > 0)
                    ++map->indicator[i][j - 1];
                if (j < map->width - 1)
                    ++map->indicator[i][j + 1];
            }
        }
    }
}

