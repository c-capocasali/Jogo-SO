#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <condition_variable>
#include <mutex>

class Semaphore {
private:
  // Atributos
  std::mutex mtx;
  std::condition_variable cv;
  int count;

public:
  // Construtor
  Semaphore(int init_count = 0) { count = init_count; }

  // Operação down
  void wait() {
    std::unique_lock<std::mutex> lock(mtx);

    // Garantir que a thread não vai acordar
    while (count == 0) {
      cv.wait(lock);
    }

    // Se saiu do while, é porque count > 0.
    count--;
  }

  // Tenta esperar sem bloquear
  bool try_wait() {
    std::unique_lock<std::mutex> lock(mtx);
    if (count > 0) {
      count--;
      return true;
    }
    return false;
  }

  // Operação UP
  void signal() {
    std::unique_lock<std::mutex> lock(mtx);
    count++;
    // acorda thread para no wait();
    cv.notify_one();
  }
};

#endif