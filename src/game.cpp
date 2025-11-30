#include "game.h"
#include "zombie_spawner.h"
#include <iostream>
#include <random>
#include <vector>

// Helper for random numbers
int getRandom(int min, int max) {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(min, max);
  return dis(gen);
}

Game::~Game() {
  if (spawner) {
    spawner->stop();
    delete spawner;
  }
}

Game::Game() : score(0), lives(3), itemsRemaining(0), running(true) {
  player.facing = RIGHT; // Default direction
  spawner = new ZombieSpawner(&player.pos);
}

void Game::init() {
  std::lock_guard<std::mutex> lock(gameMutex);

  // 1. Create Grid
  grid.resize(GRID_HEIGHT);
  for (int y = 0; y < GRID_HEIGHT; ++y) {
    grid[y] = std::string(GRID_WIDTH, SYMBOL_EMPTY);

    // Add borders
    for (int x = 0; x < GRID_WIDTH; ++x) {
      if (y == 0 || y == GRID_HEIGHT - 1 || x == 0 || x == GRID_WIDTH - 1) {
        grid[y][x] = SYMBOL_WALL;
      }
    }
  }

  // 2. Place Player (Center)
  player.pos = {GRID_WIDTH / 2, GRID_HEIGHT / 2};

  // 3. Start Spawner
  spawner->start();

  // 4. Initial Items
  spawnItems();
}

void Game::spawnItems() {
  // Note: Mutex should be locked by caller if calling from update
  for (int i = 0; i < ITEMS_BATCH_SIZE; ++i) {
    int x, y;
    do {
      x = getRandom(1, GRID_WIDTH - 2);
      y = getRandom(1, GRID_HEIGHT - 2);
    } while (grid[y][x] != SYMBOL_EMPTY);

    grid[y][x] = SYMBOL_ITEM;
  }
  itemsRemaining = ITEMS_BATCH_SIZE;
}

// Pega os zumbis do buffer do spawner
void Game::checkNewZombies() {
  Point spawnPos;

  // Tenta consumir um zumbi (retorna false se não tiver nenhum pronto)
  if (spawner->consumeSpawnPosition(spawnPos)) {
    std::lock_guard<std::mutex> lock(gameMutex);
    // Cria o objeto Zombie e adiciona ao vetor
    zombies.emplace_back(spawnPos);
  }
}

void Game::updatePlayer() {
  // Verifica se chegaram novos zumbis antes de mover
  checkNewZombies();

  std::lock_guard<std::mutex> lock(gameMutex);
  if (!running)
    return;

  Point next = getNextPosition(player.pos, player.facing);

  if (isValidMove(next)) {
    player.pos = next;
    checkItemCollection(next);
    handleCollision();
  }
}

void Game::setPlayerDirection(Direction d) {
  std::lock_guard<std::mutex> lock(gameMutex);
  player.facing = d;
}

Point Game::getNextPosition(Point current, Direction dir) {
  Point next = current;
  switch (dir) {
  case UP:
    next.y--;
    break;
  case DOWN:
    next.y++;
    break;
  case LEFT:
    next.x--;
    break;
  case RIGHT:
    next.x++;
    break;
  default:
    break;
  }
  return next;
}

bool Game::isValidMove(Point p) {
  if (p.x < 0 || p.x >= GRID_WIDTH || p.y < 0 || p.y >= GRID_HEIGHT)
    return false;
  if (grid[p.y][p.x] == SYMBOL_WALL)
    return false;

  for (auto &z : zombies) {
    if (z.getZombiePosition() == p)
      return false;
  }
  return true;
}

void Game::checkItemCollection(Point p) {
  if (grid[p.y][p.x] == SYMBOL_ITEM) {
    score += 10;
    grid[p.y][p.x] = SYMBOL_EMPTY;
    itemsRemaining--;
    if (itemsRemaining <= 0) {
      spawnItems();
    }
  }
}

void Game::handleCollision() {

  for (auto &z : zombies) {
    if (player.pos == z.getZombiePosition()) {
      std::lock_guard<std::mutex> lifeLock(livesMutex);
      lives--;
      if (lives <= 0)
        running = false;
      return;
    }
  }
}

// Atualizada para usar a classe Zombie
void Game::updateZombie(int zombieIndex) {
  Point oldPos;
  Point newPos;

  {
    std::lock_guard<std::mutex> lock(gameMutex);
    if (!running || zombieIndex >= (int)zombies.size())
      return;

    // Copia estado necessário para calcular sem lock
    oldPos = zombies[zombieIndex].getZombiePosition();

    newPos = zombies[zombieIndex].calculateNextMove(player.pos, grid);
  }

  {
    std::lock_guard<std::mutex> lock(gameMutex);

    // Verificamos se a nova posição é válida
    if (isValidMove(newPos)) {
      // Se válido, apenas verificamos colisão
      handleCollision();
    } else {
      // Se inválido (bateu noutro zumbi), revertemos para a posição antiga
      zombies[zombieIndex].setPosition(oldPos);
    }
  }
}

void Game::draw() {
  std::lock_guard<std::mutex> lock(gameMutex);

  // Move cursor to top-left
  std::cout << "\033[H";

  // Header
  std::cout << "SCORE: " << score << " | LIVES: " << lives
            << " | ZOMBIES: " << zombies.size() << "\n";

  // Draw Grid
  for (int y = 0; y < GRID_HEIGHT; ++y) {
    for (int x = 0; x < GRID_WIDTH; ++x) {
      bool dynamicDrawn = false;

      // Draw Player
      if (player.pos.x == x && player.pos.y == y) {
        std::cout << COLOR_PLAYER << SYMBOL_PLAYER << COLOR_RESET;
        dynamicDrawn = true;
      }
      // Draw Zombies
      else {
        // CORREÇÃO: Usar getZombiePosition()
        for (auto &z : zombies) {
          Point zp = z.getZombiePosition();
          if (zp.x == x && zp.y == y) {
            std::cout << COLOR_ZOMBIE << SYMBOL_ZOMBIE << COLOR_RESET;
            dynamicDrawn = true;
            break;
          }
        }
      }

      if (!dynamicDrawn) {
        if (grid[y][x] == SYMBOL_ITEM)
          std::cout << COLOR_ITEM << SYMBOL_ITEM << COLOR_RESET;
        else
          std::cout << grid[y][x];
      }
      std::cout << " "; // spacing for aspect ratio
    }
    std::cout << "\n";
  }
}

bool Game::isRunning() const { return running; }
int Game::getScore() const { return score; }
int Game::getLives() const { return lives; }
