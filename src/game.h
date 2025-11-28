#ifndef GAME_H
#define GAME_H

#include <vector>
#include <string>
#include <mutex>
#include <atomic>
#include "config.h"

struct Point {
    int x, y;
    bool operator==(const Point& other) const { return x == other.x && y == other.y; }
};

enum Direction { UP, DOWN, LEFT, RIGHT, NONE };

struct Entity {
    Point pos;
    Direction facing;
};

class Game {
public:
    Game();
    
    // Core setup
    void init();
    void spawnItems();
    
    // Actions (Thread Safe)
    void updatePlayer(); 
    void updateZombie(int zombieIndex);
    void setPlayerDirection(Direction d);
    
    // Rendering
    void draw();
    
    // State Checkers
    bool isRunning() const;
    int getScore() const;
    int getLives() const;

private:
    // Game State
    std::vector<std::string> grid;
    Entity player;
    std::vector<Entity> zombies;
    int score;
    int lives;
    int itemsRemaining;
    std::atomic<bool> running;
    
    // Synchronization
    std::mutex gameMutex; // Protects grid, lives, and positions
    std::mutex livesMutex; // Specific requirement from prompt

    // Internal Helpers
    Point getNextPosition(Point current, Direction dir);
    Point calculateBFS(Point start, Point target);
    bool isValidMove(Point p);
    void handleCollision();
    void checkItemCollection(Point p);
};

#endif