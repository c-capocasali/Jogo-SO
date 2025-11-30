#include "zombie_spawner.h"
#include "config.h"
#include "utils.h"
#include <chrono>
#include <cstdlib>

// Incializa as variáveis
ZombieSpawner::ZombieSpawner(const Point *playerPosRef)
    : items_sem(0), slots_sem(3), playerPos(playerPosRef), activeZombies(0) {
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
  std::vector<Point> corners = {
      {1, 1},                           // Canto Superior Esquerdo
      {GRID_WIDTH - 2, 1},              // Canto Superior Direito
      {1, GRID_HEIGHT - 2},             // Canto Inferior Esquerdo
      {GRID_WIDTH - 2, GRID_HEIGHT - 2} // Canto Inferior Direito
  };

  Point p;
  bool valid;

  // Tenta escolher um canto que não seja onde o jogador está
  do {
    int index = getRandom(0, 3); // Escolhe um dos 4 cantos
    p = corners[index];

    // Verifica se colide com o jogador
    valid = !(p.x == playerPos->x && p.y == playerPos->y);

  } while (!valid);

  return p;
}
// Código do produtor usando semáforos
void ZombieSpawner::producerLoop() {
  // Loop do Produtor
  while (running) {
    // Espera 10 segundos
    std::this_thread::sleep_for(std::chrono::seconds(6));
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
