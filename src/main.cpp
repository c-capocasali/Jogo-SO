#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include "game.h"

// --- Platform Specific Includes ---
#ifdef _WIN32
    #include <windows.h>
    #include <conio.h> // For _kbhit() and _getch()
#else
    #include <termios.h>
    #include <unistd.h>
    #include <fcntl.h>
#endif

// --- Cross-Platform Console Setup ---

// Enable ANSI colors for Windows 10+
void enableWindowsANSI() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
#endif
}

// Configures terminal for non-blocking raw input
void setNonBlockingInput(bool enable) {
#ifdef _WIN32
    // Windows doesn't need specific blocking configuration for _kbhit/_getch
    // But we might want to hide the cursor
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = !enable; // Hide cursor when game runs
    SetConsoleCursorInfo(consoleHandle, &info);
#else
    static struct termios oldt, newt;
    if (enable) {
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO); // Disable buffer and echo
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
    } else {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);
    }
#endif
}

// --- Thread Functions ---

// 1. Input Thread: Reads keys instantly
void inputThreadFunc(Game* game, bool* exitFlag) {
    while (!(*exitFlag) && game->isRunning()) {
#ifdef _WIN32
        // Windows Logic
        if (_kbhit()) {
            char ch = _getch();
            switch(ch) {
                case 'w': game->setPlayerDirection(UP); break;
                case 's': game->setPlayerDirection(DOWN); break;
                case 'a': game->setPlayerDirection(LEFT); break;
                case 'd': game->setPlayerDirection(RIGHT); break;
                case 'q': *exitFlag = true; break; 
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
#else
        // Linux/Mac Logic
        char ch = getchar();
        if (ch != EOF) {
            switch(ch) {
                case 'w': game->setPlayerDirection(UP); break;
                case 's': game->setPlayerDirection(DOWN); break;
                case 'a': game->setPlayerDirection(LEFT); break;
                case 'd': game->setPlayerDirection(RIGHT); break;
                case 'q': *exitFlag = true; break; 
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); 
#endif
    }
}

// 2. Zombie Thread: Calculates path and moves
void zombieThreadFunc(Game* game, int zombieIndex) {
    while (game->isRunning()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(TICK_RATE_MS));
        game->updateZombie(zombieIndex);
    }
}

// --- Main ---

int main() {
    enableWindowsANSI(); // Ensure colors work on Windows

    Game game;
    game.init();

    // Clear screen once at start
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif

    bool exitFlag = false;

    // Start Input Thread
    setNonBlockingInput(true);
    std::thread inputThread(inputThreadFunc, &game, &exitFlag);

    // Start Zombie Threads
    std::vector<std::thread> zombieThreads;
    for (int i = 0; i < ZOMBIE_COUNT; ++i) {
        zombieThreads.emplace_back(zombieThreadFunc, &game, i);
    }

    // Main Game Loop (Player Move + Render + Timer)
    auto startTime = std::chrono::steady_clock::now();
    
    while (!exitFlag && game.isRunning()) {
        // Check Time Limit
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();
        if (elapsed >= GAME_DURATION_SECONDS) break;

        // Update Player and Draw
        game.updatePlayer();
        game.draw();
        
        std::cout << "Time: " << (GAME_DURATION_SECONDS - elapsed) << "s" << std::endl;

        // Tick Sleep
        std::this_thread::sleep_for(std::chrono::milliseconds(TICK_RATE_MS));
    }

    // Cleanup
    setNonBlockingInput(false); // Restore terminal
    exitFlag = true; // Signal threads to stop

    inputThread.detach();
    for (auto& t : zombieThreads) t.detach();

#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif

    std::cout << "GAME OVER!\n";
    std::cout << "Final Score: " << game.getScore() << "\n";
    
    if (game.getLives() <= 0) std::cout << "Cause: You were eaten.\n";
    else std::cout << "Cause: Time limit reached.\n";

    return 0;
}