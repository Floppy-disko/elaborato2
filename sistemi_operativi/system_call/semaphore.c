/// @file semaphore.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione dei SEMAFORI.

#include <sys/sem.h>

#include "semaphore.h"
#include "err_exit.h"

void semOp(int semid, unsigned short sem_num, short sem_op) {
    struct sembuf sop = {
            sem_num, /* Semaphore number */
            sem_op, /* Operation to be performed */
            0 /* Operation flags */
    };

    if (semop(semid, &sop, 1) == -1)
        errExit("semop failed");
}
