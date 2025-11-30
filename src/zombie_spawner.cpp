#include "zombie_spawner.h"
#include "config.h"
#include <chrono>
#include <cstdlib>

// Incializa as variáveis
ZombieSpawner::ZombieSpawner(const Point *playerPosRef)
    : items_sem(0), slots_sem(3), playerPos(playerPosRef),
      activeZombies(1) { // Começamos com apenas 1 zumbi
  running = false;
}

ZombieSpawner::~ZombieSpawner() { stop(); }

// Cria thread para gerar zumbi
void ZombieSpawner::start() {
  running = true;
  spawnerThread = std::thread(&ZombieSpawner::producerLoop, this);
}

// Função para matar a thread
// TODO: Verificar essa função
void ZombieSpawner::stop() {
  running = false;
  if (spawnerThread.joinable()) {
    spawnerThread.join();
  }
}

// Spawna o zumbi em uma das bordas.
// Caso jogador presente em uma das bordas escolhidas, seleciona outra
Point ZombieSpawner::generateBorderPosition() {
  // Escolhe uma borda: 0=Top, 1=Bottom, 2=Left, 3=Right
  int edge = rand() % 4;
  Point p;

  do {
    edge = rand() % 4;
    switch (edge) {
    case 0: // Top
      p = {rand() % GRID_WIDTH, 0};
      break;
    case 1: // Bottom
      p = {rand() % GRID_WIDTH, GRID_HEIGHT - 1};
      break;
    case 2: // Left
      p = {0, rand() % GRID_HEIGHT};
      break;
    case 3: // Right
      p = {GRID_WIDTH - 1, rand() % GRID_HEIGHT};
      break;
    }
  } while (p.x == playerPos->x && p.y == playerPos->y);

  return p;
}

// Código do produtor usando semáforos
void ZombieSpawner::producerLoop() {
  // Loop do Produtor
  while (running) {
    // Espera 10 segundos
    std::this_thread::sleep_for(std::chrono::seconds(10));
    if (!running)
      break;

    // Verifica limite de zumbis
    if (activeZombies >= ZOMBIE_COUNT) {
      continue;
    }

    // Posição de spawn do zumbi
    Point spawnPos = generateBorderPosition();

    // Decrementa vazios
    slots_sem.wait();

    // Entra e sai da região crítica para adicionar zumbis na fila
    {
      std::lock_guard<std::mutex> lock(queueMutex);
      spawnQueue.push(spawnPos);
    }

    // Incrementa o contador de cheios e sinaliza consumidor
    items_sem.signal();

    activeZombies++;
  }
}

// Código do consumidor usando semáforos
bool ZombieSpawner::consumeSpawnPosition(Point &p) {

  // Tenta pegar sem bloquear o jogo
  if (items_sem.try_wait()) {
    std::lock_guard<std::mutex> lock(queueMutex);

    p = spawnQueue.front();
    spawnQueue.pop();

    slots_sem.signal(); // Libera espaço na fila
    return true;
  }
  return false;
}

// Retorna a quantidade de zumbis presentes no jogo
int ZombieSpawner::getCurrentZombieCount() { return activeZombies; }
