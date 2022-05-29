/// @file semaphore.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione dei SEMAFORI.

#include <sys/sem.h>
#include <errno.h>

#include "semaphore.h"
#include "err_exit.h"

///@param wait a 0 se il semaforo è bloccante, a 1 sed non è bloccante
int semOp(int semid, unsigned short sem_num, short sem_op, int wait) {
    struct sembuf sop = {
            sem_num, /* Semaphore number */
            sem_op, /* Operation to be performed */
            (wait) ? 0 : IPC_NOWAIT /* Operation flags */
    };

    errno=0;
    if (semop(semid, &sop, 1) == -1)
        if(errno==EAGAIN)  //semplicemente dovrei bloccarmi ma IPC_NOWAIT mi permette di andare avanti
            return -1;
        else
            errExit("semop failed");

    else
        return 0;
}
