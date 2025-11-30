#include "config.h"
#include "game.h"

using namespace std;

class Zombie {
private:
  Point zCoordinates;

public:
  // Construtor da classe
  Zombie(Point position) { zCoordinates = position; }

  // Obtem a posição atual do zumbi
  Point getZombiePosition();

  // Calcula o bfs
  Point calculateBFS(Point start, Point target, vector<string> grid);

  // Atualiza a posição do zombi
  Point updateZombiePosition(int zombieIndex);
};
