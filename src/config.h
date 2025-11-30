#ifndef CONFIG_H
#define CONFIG_H

#include <string.h>

// --- Game Settings ---
const int GRID_WIDTH = 20;
const int GRID_HEIGHT = 20;
const int GAME_DURATION_SECONDS = 60; // Game ends after this time
const int TICK_RATE_MS = 500;         // Speed of movement
const int ZOMBIE_COUNT = 3;
const int ITEMS_BATCH_SIZE = 5;

// --- Symbols ---
const char SYMBOL_PLAYER = 'P';
const char SYMBOL_ZOMBIE = 'Z';
const char SYMBOL_ITEM = '$';
const char SYMBOL_WALL = '#';
const char SYMBOL_EMPTY = '.';

// --- Colors (ANSI Escape Codes) ---
const char *const COLOR_RESET = "\033[0m";
const char *const COLOR_PLAYER = "\033[1;32m"; // Green
const char *const COLOR_ZOMBIE = "\033[1;31m"; // Red
const char *const COLOR_ITEM = "\033[1;33m";   // Yellow

struct Point {
  int x, y;
  bool operator==(const Point &other) const {
    return x == other.x && y == other.y;
  }
};

enum Direction { UP, DOWN, LEFT, RIGHT, NONE };

struct Entity {
  Point pos;
  Direction facing;
};
#endif
