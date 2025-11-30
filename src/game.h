#ifndef GAME_H
#define GAME_H

#include "config.h"
#include "zombie.h"
#include "zombie_spawner.h"
#include <atomic>
#include <mutex>
#include <string>
#include <vector>

class Game {
public:
  Game();
  ~Game();

  // Core setup
  void init();
  void spawnItems();

  // Actions (Thread Safe)
  void updatePlayer();
  void updateZombie(int zombieIndex);
  void checkNewZombies();
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
  std::vector<Zombie> zombies;
  ZombieSpawner *spawner;
  int score;
  int lives;
  int itemsRemaining;
  std::atomic<bool> running;

  // Synchronization
  std::mutex gameMutex;  // Protects grid, lives, and positions
  std::mutex livesMutex; // Specific requirement from prompt

  // Internal Helpers
  Point getNextPosition(Point current, Direction dir);

  bool isValidMove(Point p);
  void handleCollision();
  void checkItemCollection(Point p);
};

#endif
