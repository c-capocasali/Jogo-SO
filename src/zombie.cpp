#include "zombie.h"
#include <queue>
#include <vector>

using namespace std;

// Calcula o próximo movimento do zumbi
Point Zombie::calculateNextMove(Point target, const vector<string> &grid) {
  Point nextStep = calculateBFS(zCoordinates, target, grid);
  return nextStep;
}

// Função que calcula o BFS
Point Zombie::calculateBFS(Point start, Point target,
                           const vector<string> &grid) {
  // Se já está no alvo, não move
  if (start.x == target.x && start.y == target.y)
    return start;

  int dx[] = {0, 0, -1, 1}; // Cima, Baixo, Esquerda, Direita
  int dy[] = {-1, 1, 0, 0};

  queue<Point> q;
  q.push(start);

  // Matrizes para rastrear caminh
  vector<vector<Point>> parent(GRID_HEIGHT,
                               vector<Point>(GRID_WIDTH, {-1, -1}));
  vector<vector<bool>> visited(GRID_HEIGHT, vector<bool>(GRID_WIDTH, false));

  visited[start.y][start.x] = true;
  bool found = false;

  while (!q.empty()) {
    Point curr = q.front();
    q.pop();

    if (curr.x == target.x && curr.y == target.y) {
      found = true;
      break;
    }

    for (int i = 0; i < 4; i++) {
      Point next = {curr.x + dx[i], curr.y + dy[i]};

      // Verifica limites e paredes
      if (next.x >= 0 && next.x < GRID_WIDTH && next.y >= 0 &&
          next.y < GRID_HEIGHT && grid[next.y][next.x] != SYMBOL_WALL &&
          !visited[next.y][next.x]) {

        visited[next.y][next.x] = true;
        parent[next.y][next.x] = curr;
        q.push(next);
      }
    }
  }

  if (!found)
    return start; // Sem caminho

  // Backtracking para achar o primeiro passo
  Point curr = target;
  while (!(parent[curr.y][curr.x].x == start.x &&
           parent[curr.y][curr.x].y == start.y)) {
    curr = parent[curr.y][curr.x];

    if (curr.x == -1)
      return start;
  }

  return curr;
}
