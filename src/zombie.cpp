#include "zombie.h"
#include <queue>

using namespace std;

Point Zombie::getZombiePosition() { return zCoordinates; };

Point Zombie::calculateBFS(Point start, Point target, vector<string> grid) {
  // 4 possible movements
  int dx[] = {0, 0, -1, 1};
  int dy[] = {-1, 1, 0, 0};

  std::queue<Point> q;
  q.push(start);

  vector<std::vector<Point>> parent(GRID_HEIGHT,
                                    std::vector<Point>(GRID_WIDTH, {-1, -1}));
  vector<std::vector<bool>> visited(GRID_HEIGHT,
                                    std::vector<bool>(GRID_WIDTH, false));

  visited[start.y][start.x] = true;

  bool found = false;

  while (!q.empty()) {
    Point curr = q.front();
    q.pop();

    if (curr == target) {
      found = true;
      break;
    }

    for (int i = 0; i < 4; i++) {
      Point next = {curr.x + dx[i], curr.y + dy[i]};

      // Check boundaries and walls (treat zombies as obstacles for BFS?)
      // For simplicity, zombies only see walls as obstacles in pathfinding
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
    return start; // No path found

  // Backtrack to find the first step
  Point curr = target;
  while (!(parent[curr.y][curr.x] == start)) {
    curr = parent[curr.y][curr.x];
    if (curr.x == -1)
      return start; // Safety break
  }
  return curr;
}
