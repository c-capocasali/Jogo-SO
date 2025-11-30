#include "game.h"
#include "zombie_spawner.h"
#include "map.h"
#include "utils.h"
#include <iostream>
#include <vector>

Game::~Game() {
  if (spawner) {
    spawner->stop();
    delete spawner;
  }
}

Game::Game() : score(0), lives(3), itemsRemaining(0), running(true) {
  player.facing = RIGHT; // Direção inicial
  spawner = new ZombieSpawner(&player.pos);
}

void Game::init() {
  std::lock_guard<std::mutex> lock(gameMutex);

  // 1. Criar um grid aleatório
  grid = generateRandomMap();

  // 2. Colocar o player no centro
  player.pos = {GRID_WIDTH / 2, GRID_HEIGHT / 2};

  // 3. Iniciar o spawner de zumbis
  spawner->start();

  // 4. Colocar os itens iniciais
  spawnItems();
}

void Game::spawnItems() {
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
    playSoundEffect(0); // Som de coleta
    score += 10;
    grid[p.y][p.x] = SYMBOL_EMPTY;
    itemsRemaining--;
    if (itemsRemaining <= 0) {
      spawnItems();
    }
  }
}

void Game::handleDamaging() {
  std::lock_guard<std::mutex> lifeLock(livesMutex);
  playSoundEffect(1); // Som de dano
  lives--;
  if (lives <= 0)
    running = false;
  return;
}

// Atualiza a posição do zumbi
void Game::updateZombie(int zombieIndex) {
  Point newPos;

  {
    std::lock_guard<std::mutex> lock(gameMutex);
    if (!running || zombieIndex >= (int)zombies.size())
      return;

    // Calcula a próxima posição, mas ainda não move o zumbi
    newPos = zombies[zombieIndex].calculateNextMove(player.pos, grid);
  }

  {
    // Entrando na região crítica
    std::lock_guard<std::mutex> lock(gameMutex);

    // Verifica se a nova posição é válida
    if (isValidMove(newPos)) {
      // Se válido, movemos o zumbi explicitamente
      if (newPos.x == player.pos.x && newPos.y == player.pos.y) {
        handleDamaging();
        return;
      }
      zombies[zombieIndex].setPosition(newPos);
    }
  }
}

void Game::draw() {
  std::lock_guard<std::mutex> lock(gameMutex);

  // Move o cursor para o topo esquerdo
  std::cout << "\033[H";

  // Imprime o header
  std::cout << "SCORE: " << score << " | LIVES: " << lives
            << " | ZOMBIES: " << zombies.size() << "\n";

  // Desenha o grid
  for (int y = 0; y < GRID_HEIGHT; ++y) {
    for (int x = 0; x < GRID_WIDTH; ++x) {
      bool dynamicDrawn = false;

      // Desenha o Player
      if (player.pos.x == x && player.pos.y == y) {
        std::cout << COLOR_PLAYER << SYMBOL_PLAYER << COLOR_RESET;
        dynamicDrawn = true;
      }
      // Desenha os Zumbis
      else {
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
      std::cout << " ";
    }
    std::cout << "\n";
  }
}

bool Game::isRunning() const { return running; }
int Game::getScore() const { return score; }
int Game::getLives() const { return lives; }
