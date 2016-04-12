//
// Created by Kelvin on 2016/4/12.
//

#ifndef MINESWEEPER_MODEL_H
#define MINESWEEPER_MODEL_H

#include<stdbool.h>

#define MAP_STATUS_DEFAULT              0
#define MAP_STATUS_VISITED              1
#define MAP_STATUS_MARK_MINE            2
#define MAP_STATUS_MARK_NOT_DETERMINED  3

typedef struct {
    int width, height, totalMines, remainMines, remainBlocks;
    bool firstVisit;
    bool **field;
    unsigned char **indicator;
    unsigned char **status;
} Map;

Map *createMap(int width, int height, int mines);

void printMap(Map *map);

void destroyMap(Map *map);

int visitMap(Map *map, int row, int column);

int quickVisitMap(Map *map, int row, int column);

void markMap(Map *map, int row, int column);

#endif //MINESWEEPER_MODEL_H
