/// @file shared_memory.h
/// @brief Contiene la definizioni di variabili e funzioni
///         specifiche per la gestione della MEMORIA CONDIVISA.

#pragma once

#include <stdlib.h>

//creo shared memory
int alloc_shared_memory(key_t shmKey);

//attacco shered memory
void *get_shared_memory(int shmid, int shmflg);


//stacco chered memory
void free_shared_memory(void *ptr_sh);

//elimino shared memory
void remove_shared_memory(int shmid);
