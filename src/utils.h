#ifndef UTILS_H
#define UTILS_H

// Gera um número aleatório entre min e max (inclusivo)
// Implementação thread-safe
int getRandom(int min, int max);

// Toca um efeito sonoro baseado no tipo
// type = 0 -> Som de coleta de item
// type = 1 -> Som de dano ao player
void playSoundEffect(int type);

#endif