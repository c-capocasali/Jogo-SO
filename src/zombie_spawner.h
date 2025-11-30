#ifndef ZOMBIE_SPAWNER_H
#define ZOMBIE_SPAWNER_H

#include "config.h"
#include "semaphore.h"
#include <atomic>
#include <mutex>
#include <queue>
#include <thread>

using namespace std;

class ZombieSpawner {
public:
  // Recebe referência do playerPosition para calcular spawn longe dele
  ZombieSpawner(const Point *playerPosRef);
  ~ZombieSpawner();

  // Inicia a thread produtora
  void start();

  // Para a thread
  void stop();

  // Retorna true se houver um zumbi para spawnar e preenche
  bool consumeSpawnPosition(Point &p);

  // Retorna quantos zumbis ativos existem
  int getCurrentZombieCount();

private:
  void producerLoop();
  Point generateBorderPosition();

  queue<Point> spawnQueue; // Buffer compartilhado

  // Sincronização
  Semaphore items_sem; // Conta itens na fila (Full)
  Semaphore slots_sem; // Conta espaços vazios (Empty)
  mutex queueMutex;

  // Controle da Thread
  thread spawnerThread;
  atomic<bool> running;

  // Estado do Jogo
  const Point *playerPos;    // Ponteiro de leitura para posição do player
  atomic<int> activeZombies; // Controla limite de 3
};

#endif
