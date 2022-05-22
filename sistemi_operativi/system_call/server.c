/// @file sender_manager.c
/// @brief Contiene l'implementazione del sender_manager.

#include "err_exit.h"
#include "defines.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "fifo.h"

int fifo1=-1;
int fifo2=-1;
int msqid=-1;

void sigHandler(int sig){
    if(sig==SIGINT){
        if(close(fifo1)==-1 || close(fifo2)==-1)
            errExit("Closing fifos FDs failed");
        if(unlink(PATH_FIFO1)==-1 || unlink(PATH_FIFO2)==-1)
            errExit("Unlinking fifos failed");

        if (msgctl(msqid, IPC_RMID, NULL) == -1)
            errExit("msgctl failed");

    }
}

int main(int argc, char *argv[]) {

    //creo le key
    key_t msgq_k = ftok(".", KEY_MSGQ);
    if(msgq_k==-1)
        errExit("ftok msgq failed");
    key_t shdmem_k = ftok(".", KEY_SHDMEM);
    if(shdmem_k==-1)
        errExit("ftok shdmem failed");

    //creo fifo1 in lettura
    if (mkfifo(PATH_FIFO1, S_IWUSR | S_IRUSR) == -1)
        errExit("mkfifo failed");
    fifo1 = open(PATH_FIFO1, O_RDONLY);
    if (fifo1 == -1)
        errExit("open fifo1 failed");

    //creo fifo2 in lettura
    if (mkfifo(PATH_FIFO2, S_IWUSR | S_IRUSR) == -1)
        errExit("mkfifo failed");
    fifo2 = open(PATH_FIFO2, O_RDONLY);
    if (fifo2 == -1)
        errExit("open fifo2 failed");

    //creo msg queue
    msqid = msgget(msgq_k, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (msqid == -1)
        errExit("msgget failled");

    //creo shd memory
    int shmidServer = alloc_shared_memory(shdmem_k);
    void *SHDMEMbuffer = get_shared_memory(shmidServer, 0);

    if (signal(SIGINT, sigHandler) == SIG_ERR)
        errExit("change signal handler failed");

    while(1) {
        //aspetta numero n di file

    }

    return 0;
}

