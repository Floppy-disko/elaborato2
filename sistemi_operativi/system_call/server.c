/// @file sender_manager.c
/// @brief Contiene l'implementazione del sender_manager.

#include "defines.h"

int partsReceived;

void sigHandler(int sig){
    if(sig==SIGINT){     //chiudo ed elimino le ipc
        if(close(fifo1)==-1 || close(fifo2)==-1)
            errExit("Closing fifos FDs failed");
        if(unlink(fifo1Path)==-1 || unlink(fifo2Path)==-1)
            errExit("Unlinking fifos failed");

        if (msgctl(msqid, IPC_RMID, NULL) == -1)
            errExit("msgctl failed");

        free_shared_memory(shdmemBuffer);
        remove_shared_memory(shdmemid);

        if (semctl(semShdmemid, 0, IPC_RMID) == -1)
            errExit("semctl IPC_RMID failed");

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

    key_t semShdmem_k = ftok(getenv("HOME"), KEY_SEM_SHDMEM);
    if(shdmem_k==-1)
        errExit("ftok semShdmem failed");

    //creo msg queue
    msqid = msgget(msgq_k, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (msqid == -1)
        errExit("msgget failled");

    //creo shd memory
    int shdmSize = sizeof(struct shdmemStructure); //alloco spazio per 50 messagi più il vettore di supporto
    shdmemid = alloc_shared_memory(shdmem_k, shdmSize);
    shdmemBuffer = (struct shdmemStructure*)get_shared_memory(shdmemid, 0);

    shdmemBuffer->in = 0; //all inizio tutte le celle sono vuote
    shdmemBuffer->out = 0;

    semShdmemid = semget(semShdmem_k, 3, S_IRUSR | S_IWUSR | IPC_CREAT);
    if (semShdmemid == -1)
        errExit("semget failed");

    unsigned short semShdmemInitVal[] = {1, 0, MSG_NUMBER_MAX};
    union semun arg;
    arg.array = semShdmemInitVal;

    //setta i semafori 0)mutex=1, 1)full=0, 2)empty=50
    if (semctl(semShdmemid, 0, SETALL, arg))
        errExit("semctl SETALL failed");

    //creo fifo1
    sprintf(fifo1Path, "%s/%s", getenv("HOME"), PATH_FIFO1);  //concateno il nome del file alla cartella home
    if (mkfifo(fifo1Path, S_IWUSR | S_IRUSR) == -1)
        errExit("mkfifo failed");

    //creo fifo2
    sprintf(fifo2Path, "%s/%s", getenv("HOME"), PATH_FIFO2);
    if (mkfifo(fifo2Path, S_IWUSR | S_IRUSR) == -1)
        errExit("mkfifo failed");

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
        printf("\nLeggo il numero di n_file da fifo1");
        char n_fileString[4];
        int br;

        do {  //provo a leggere finchè non trovo qualcosa di diverso dal carattere terminatore
            br = read(fifo1, n_fileString, sizeof(n_fileString));
            if (br == -1)
                errExit("Read failed");
        } while(br==0);

        n_file = atoi(n_fileString);

        printf("\nHo letto: %s, invio conferma a client_0\n", n_fileString);
        fflush(stdout);
        write_in_shdmem(shdmemBuffer, "", "Conferma ricevimento n_file");

        printf("\nMi metto in ascolto delle parti di file");
    }

    return 0;
}

