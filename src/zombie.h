#ifndef ZOMBIE_H
#define ZOMBIE_H

#include "config.h"
#include <string>
#include <vector>

class Zombie {
private:
  Point zCoordinates;

public:
  // Construtor
  Zombie(Point position) { zCoordinates = position; }

  // Retorna a posição atual
  Point getZombiePosition() { return zCoordinates; }

  // Define uma nova posição
  void setPosition(Point p) { zCoordinates = p; }

  // Retorna a próxima posição
  Point calculateNextMove(Point target, const std::vector<std::string> &grid);

private:
  // Usa BFS para calcular a posição do jogador
  Point calculateBFS(Point start, Point target, const std::vector<std::string> &grid);
};

#endif