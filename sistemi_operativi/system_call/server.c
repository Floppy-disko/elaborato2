/// @file sender_manager.c
/// @brief Contiene l'implementazione del sender_manager.

#include "defines.h"

void sigHandler(int sig){
    if(sig==SIGINT){     //chiudo ed elimino le ipc
        if(close(fifo1)==-1 || close(fifo2)==-1)
            errExit("Closing fifos FDs failed");
        if(unlink(fifo1Path)==-1 || unlink(fifo2Path)==-1)
            errExit("Unlinking fifos failed");

        if (msgctl(msqid, IPC_RMID, NULL) == -1)
            errExit("msgctl failed");

        free_shared_memory(shdememBuffer);
        remove_shared_memory(shdmemid);

        exit(0);
    }
}

int main(int argc, char *argv[]) {

    //creo le key
    key_t msgq_k = ftok(getenv("HOME"), KEY_MSGQ);
    if(msgq_k==-1)
        errExit("ftok msgq failed");
    key_t shdmem_k = ftok(getenv("HOME"), KEY_SHDMEM);
    if(shdmem_k==-1)
        errExit("ftok shdmem failed");

    //creo fifo1
    sprintf(fifo1Path, "%s/%s", getenv("HOME"), PATH_FIFO1);  //concateno il nome del file alla cartella home
    if (mkfifo(fifo1Path, S_IWUSR | S_IRUSR) == -1)
        errExit("mkfifo failed");

    //creo fifo2
    sprintf(fifo2Path, "%s/%s", getenv("HOME"), PATH_FIFO2);
    if (mkfifo(fifo2Path, S_IWUSR | S_IRUSR) == -1)
        errExit("mkfifo failed");

    //creo msg queue
    msqid = msgget(msgq_k, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (msqid == -1)
        errExit("msgget failled");

    //creo shd memory
    int shdmSize = sizeof(struct bareMessage)*MSG_MAX + sizeof(shdmControl); //alloco spazio per 50 messagi più il vettore di supporto
    shdmemid = alloc_shared_memory(shdmem_k, shdmSize);
    shdememBuffer = get_shared_memory(shdmemid, 0);

    //ora apro le fifo in lettura
    fifo1 = open(fifo1Path, O_RDONLY);
    if (fifo1 == -1)
        errExit("open fifo1 failed");
    fifo2 = open(fifo2Path, O_RDONLY);
    if (fifo2 == -1)
        errExit("open fifo2 failed");

    if (signal(SIGINT, sigHandler) == SIG_ERR)
        errExit("change signal handler failed");

    while(1) {
        //aspetta numero n di file

    }

    return 0;
}

