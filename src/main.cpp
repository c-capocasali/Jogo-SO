#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include "game.h"

// --- Includes específicos de SO ---
#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
#else
    #include <termios.h>
    #include <unistd.h>
    #include <fcntl.h>
#endif

// Permite uso de cores ANSI no Windows
void enableWindowsANSI() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
#endif
}

// Configura o terminal para input não bloqueante
void setNonBlockingInput(bool enable) {
#ifdef _WIN32
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = !enable; // Esconde o cursor durante o jogo
    SetConsoleCursorInfo(consoleHandle, &info);
#else
    static struct termios oldt, newt;
    if (enable) {
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO); // Desabilita buffer e echo
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

// 1. Thread de Input: Captura entrada do usuário
void inputThreadFunc(Game* game, bool* exitFlag) {
    while (!(*exitFlag) && game->isRunning()) {
#ifdef _WIN32
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

// 2. Thread de Zumbi: Atualiza movimento do zumbi
void zombieThreadFunc(Game* game, int zombieIndex) {
    while (game->isRunning()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(int(TICK_RATE_MS / ZOMBIE_SPEED_MODIFIER)));
        game->updateZombie(zombieIndex);
    }
}

int main() {
    enableWindowsANSI();

    Game game;
    game.init();

    // Limpa a tela antes de começar
    std::cout << "\033[H\033[2J";

    bool exitFlag = false;

    // Começa a thread de entradas
    setNonBlockingInput(true);
    std::thread inputThread(inputThreadFunc, &game, &exitFlag);

    // Começa as threads de zumbis
    std::vector<std::thread> zombieThreads;
    for (int i = 0; i < ZOMBIE_COUNT; ++i) {
        zombieThreads.emplace_back(zombieThreadFunc, &game, i);
    }

    // Loop Principal (Movimento do Player + Render + Timer)
    auto startTime = std::chrono::steady_clock::now();
    
    while (!exitFlag && game.isRunning()) {
        // Verifica tempo decorrido
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();
        if (elapsed >= GAME_DURATION_SECONDS) break;

        // Atualiza player e desenha o jogo
        game.updatePlayer();
        game.draw();
        
        std::cout << "Time: " << (GAME_DURATION_SECONDS - elapsed) << "s" << std::endl;

        // Tick Sleep
        std::this_thread::sleep_for(std::chrono::milliseconds(TICK_RATE_MS));
    }

    // Limpeza ao sair
    setNonBlockingInput(false); // Devolve o terminal ao estado normal
    exitFlag = true; // Sinaliza threads para sair

    inputThread.detach();
    for (auto& t : zombieThreads) t.detach();

    std::cout << "\033[H\033[2J";

    std::cout << "GAME OVER!\n";
    std::cout << "Final Score: " << game.getScore() << "\n";
    
    if (game.getLives() <= 0) std::cout << "Cause: You were eaten.\n";
    else std::cout << "Cause: Time limit reached.\n";

    // Pergunta se quer jogar de novo ou sair
    std::cout << "Play again? (y/n): ";
    char choice;
    std::cin >> choice;
    if (choice == 'y' || choice == 'Y') {
        main(); // Reinicia o jogo
    }

    return 0;
}