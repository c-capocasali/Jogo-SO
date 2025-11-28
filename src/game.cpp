#include "game.h"
#include <iostream>
#include <queue>
#include <algorithm>
#include <random>
#include <map>

// Helper for random numbers
int getRandom(int min, int max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(min, max);
    return dis(gen);
}

Game::Game() : score(0), lives(3), itemsRemaining(0), running(true) {
    player.facing = RIGHT; // Default direction
}

void Game::init() {
    std::lock_guard<std::mutex> lock(gameMutex);
    
    // 1. Create Grid
    grid.resize(GRID_HEIGHT);
    for (int y = 0; y < GRID_HEIGHT; ++y) {
        grid[y] = std::string(GRID_WIDTH, SYMBOL_EMPTY);
        
        // Add borders
        for(int x = 0; x < GRID_WIDTH; ++x) {
            if (y == 0 || y == GRID_HEIGHT - 1 || x == 0 || x == GRID_WIDTH - 1) {
                grid[y][x] = SYMBOL_WALL;
            }
        }
    }

    // 2. Place Player (Center)
    player.pos = {GRID_WIDTH / 2, GRID_HEIGHT / 2};

    // 3. Place Zombies (Random spots not occupied)
    for (int i = 0; i < ZOMBIE_COUNT; ++i) {
        Entity z;
        do {
            z.pos = {getRandom(1, GRID_WIDTH - 2), getRandom(1, GRID_HEIGHT - 2)};
        } while (z.pos == player.pos); // Don't spawn on player
        zombies.push_back(z);
    }

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

void Game::setPlayerDirection(Direction d) {
    // Atomic or simple assignment is fine for enums, but strictly:
    std::lock_guard<std::mutex> lock(gameMutex);
    player.facing = d;
}

Point Game::getNextPosition(Point current, Direction dir) {
    Point next = current;
    switch (dir) {
        case UP:    next.y--; break;
        case DOWN:  next.y++; break;
        case LEFT:  next.x--; break;
        case RIGHT: next.x++; break;
        default: break;
    }
    return next;
}

bool Game::isValidMove(Point p) {
    if (p.x < 0 || p.x >= GRID_WIDTH || p.y < 0 || p.y >= GRID_HEIGHT) return false;
    if (grid[p.y][p.x] == SYMBOL_WALL) return false;
    
    // Check against other zombies (Zombies shouldn't stack)
    for (const auto& z : zombies) {
        if (z.pos == p) return false;
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
    // Check if player touched any zombie
    for (const auto& z : zombies) {
        if (player.pos == z.pos) {
            std::lock_guard<std::mutex> lifeLock(livesMutex);
            lives--;
            if (lives <= 0) running = false;
            
            // Optional: Reset positions on hit? 
            // For now, we just take damage and keep going (grace period logic could be added here)
            return; 
        }
    }
}

void Game::updatePlayer() {
    std::lock_guard<std::mutex> lock(gameMutex);
    if (!running) return;

    Point next = getNextPosition(player.pos, player.facing);

    if (isValidMove(next)) {
        player.pos = next;
        checkItemCollection(next);
        handleCollision();
    }
}

// Simple BFS Pathfinding
Point Game::calculateBFS(Point start, Point target) {
    // 4 possible movements
    int dx[] = {0, 0, -1, 1};
    int dy[] = {-1, 1, 0, 0};

    std::queue<Point> q;
    q.push(start);
    
    std::vector<std::vector<Point>> parent(GRID_HEIGHT, std::vector<Point>(GRID_WIDTH, {-1, -1}));
    std::vector<std::vector<bool>> visited(GRID_HEIGHT, std::vector<bool>(GRID_WIDTH, false));
    
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
            if (next.x >= 0 && next.x < GRID_WIDTH && 
                next.y >= 0 && next.y < GRID_HEIGHT && 
                grid[next.y][next.x] != SYMBOL_WALL && 
                !visited[next.y][next.x]) {
                
                visited[next.y][next.x] = true;
                parent[next.y][next.x] = curr;
                q.push(next);
            }
        }
    }

    if (!found) return start; // No path found

    // Backtrack to find the first step
    Point curr = target;
    while (!(parent[curr.y][curr.x] == start)) {
        curr = parent[curr.y][curr.x];
        if (curr.x == -1) return start; // Safety break
    }
    return curr;
}

void Game::updateZombie(int zombieIndex) {
    std::lock_guard<std::mutex> lock(gameMutex);
    if (!running) return;

    Point nextStep = calculateBFS(zombies[zombieIndex].pos, player.pos);
    
    // Only move if valid (e.g., don't step on another zombie)
    if (isValidMove(nextStep)) {
        zombies[zombieIndex].pos = nextStep;
        handleCollision();
    }
}

void Game::draw() {
    std::lock_guard<std::mutex> lock(gameMutex);
    
    // Move cursor to top-left (flicker reduction) instead of clearing everything
    std::cout << "\033[H"; 

    // Header
    std::cout << "SCORE: " << score << " | LIVES: " << lives << " | TIME REMAINING\n";
    
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
                for (const auto& z : zombies) {
                    if (z.pos.x == x && z.pos.y == y) {
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