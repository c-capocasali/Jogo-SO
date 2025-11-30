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

  // Setup principal
  void init();
  void spawnItems();

  // Ações
  void updatePlayer();
  void updateZombie(int zombieIndex);
  void checkNewZombies();
  void setPlayerDirection(Direction d);

  // Renderização
  void draw();

  // Checagens de Estado
  bool isRunning() const;
  int getScore() const;
  int getLives() const;

private:
  // Estado de jogo
  std::vector<std::string> grid;
  Entity player;
  std::vector<Zombie> zombies;
  ZombieSpawner *spawner;
  int score;
  int lives;
  int itemsRemaining;
  std::atomic<bool> running;

  // Sincronização
  std::mutex gameMutex;  // Protege grid, vidas, e posições
  std::mutex livesMutex;

  // Helpers
  Point getNextPosition(Point current, Direction dir);

  bool isValidMove(Point p);
  void handleCollision();
  void checkItemCollection(Point p);
};

#endif
