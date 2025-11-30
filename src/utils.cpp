#include "utils.h"
#include <random>
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>

// Headers específicos de SO
#ifdef _WIN32
  #include <windows.h>
#else
  #include <unistd.h>
#endif

int getRandom(int min, int max) {
  // Static pra inicializar só uma vez por programa/thread
  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(min, max);
  return dis(gen);
}

// Evita sobreposição de muitos sons (thread explosion)
static std::atomic<bool> isPlaying(false);

void playSoundEffect(int type) {
  // Se já tiver um som tocando, ignoramos o novo para não "engarrafar" threads
  if (isPlaying) return;

  std::thread([type]() {
    isPlaying = true;

    #ifdef _WIN32
      if (type == 0) { // Item
        Beep(1500, 100);
        Beep(2000, 100);
      } else {         // Dano
        Beep(200, 400);
      }
    #else
      if (type == 0) { // Item
        std::cout << "\033[10;1500]\033[11;100]\a" << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      } else {         // Dano
        std::cout << "\033[10;200]\033[11;400]\a" << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(400));
      }
      
      std::cout << "\033[10;750]\033[11;100]" << std::flush;
    #endif

    isPlaying = false;
  }).detach();
}