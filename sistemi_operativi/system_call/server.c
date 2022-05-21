/// @file sender_manager.c
/// @brief Contiene l'implementazione del sender_manager.

#include "err_exit.h"
#include "defines.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "fifo.h"


int main(int argc, char *argv[]) {

    //creo path fifo1 e fifo2
    char *pathF1 = fifo1;
    char *pathF2 = fifo2;

    //creo le key
    key_t msgq_k = ftok(".", KEY_MSGQ);
    key_t shdmem_k = ftok(".", KEY_SHDMEM);

    //creo fifo1 in lettura
    if (mkfifo(pathF1, S_IWUSR | S_IRUSR) == -1)
        errExit("mkfifo failed");
    int fdfifo1 = open(pathF1, O_RDONLY);
    if (fdfifo1 == -1)
        errExit("open fifo1 failed");

    //creo fifo2 in lettura
    if (mkfifo(pathF2, S_IWUSR | S_IRUSR) == -1)
        errExit("mkfifo failed");
    int fdfifo2 = open(pathF2, O_RDONLY);
    if (fdfifo2 == -1)
        errExit("open fifo2 failed");

    //creo msg queue
    int msqid = msgget(msgq_k, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (msqid == -1)
        errExit("msgget failled");

    //creo shd memory
    int shmidServer = alloc_shared_memory(shdmem_k);
    int *SHDMEMbuffer = get_shared_memory(shmidServer, 0);


    return 0;
}

