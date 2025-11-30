#include "utils.h"
#include <random>

int getRandom(int min, int max) {
  // Static pra inicializar só uma vez por programa/thread
  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(min, max);
  return dis(gen);
}