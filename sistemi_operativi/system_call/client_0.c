/// @file client.c
/// @brief Contiene l'implementazione del client.

#include "defines.h"

void sigHandler(int sig) { //serve solo per interrompere la pause
    if(sig==SIGUSR1) {
        if(close(fifo1)==-1 || close(fifo2)==-1)
            errExit("Closing fifos FDs failed");

        free_shared_memory(shdememBuffer);
        exit(0);
    }
}

int main(int argc, char *argv[]) {

    //  ***** CONTROLLO CORRETEZZA INPUT: ./Client_0 <HOME>/myDir/ *****
    if (argc != 2) {
        char buffer[] = "\nUsage: ./client <HOME>/myDir"; //modifcare messaggio se neccessario
        if (write(STDOUT_FILENO, buffer, sizeof(buffer)) == -1)
            errExit("write on STDOUT failed");
        return 1;
    }

    char newDir[FILE_PATH_MAX];
    strcpy(newDir, argv[1]);

    //lato client apertura di fifo, mssgqueue, shared memory...VA FATTA DAI CHILD?

    key_t msgq_k = ftok(getenv("HOME"), KEY_MSGQ);
    if(msgq_k==-1)
        errExit("ftok msgq failed");
    key_t shdmem_k = ftok(getenv("HOME"), KEY_SHDMEM);
    if(shdmem_k==-1)
        errExit("ftok shdmem failed");

    //prendo fifo1 creata da server
    sprintf(fifo1Path, "%s/%s", getenv("HOME"), PATH_FIFO1);  //concateno il nome del file alla cartella home
    fifo1 = open(fifo1Path, O_WRONLY);
    if (fifo1 == -1)
        errExit("open fifo1 failed");

    //prendo fifo2 creata da server
    sprintf(fifo2Path, "%s/%s", getenv("HOME"), PATH_FIFO2);
    fifo2 = open(fifo2Path, O_WRONLY);
    if (fifo2 == -1)
        errExit("open fifo2 failed");

    //prendo msg queue creata da server
    msqid = msgget(msgq_k, S_IRUSR | S_IWUSR);
    if (msqid == -1)
        errExit("msgget failled");

    //ottengo shd memory creata da server
    shdmemid = alloc_shared_memory(shdmem_k, 0);
    shdememBuffer = get_shared_memory(shdmemid, 0);


    //  ***** SETTO SEGNALI *****
    sigset_t SigSet;
    if (sigfillset(&SigSet) == -1)
        errExit("filling mySet fail");
    if (sigdelset(&SigSet, SIGINT) == -1)
        errExit("deleting mySet fail");
    if (sigdelset(&SigSet, SIGUSR1) == -1)
        errExit("deleting mySet fail");
    if (sigprocmask(SIG_SETMASK, &SigSet, NULL) == -1)
        errExit("mask fail");
    //setto gli handler
    if (signal(SIGINT, sigHandler) == SIG_ERR)
        errExit("change signal handler failed");
    if (signal(SIGUSR1, sigHandler) == SIG_ERR)
        errExit("change signal handler failed");

    //loop con il codice vero e proprio del main
    while (1) {
        pause(); //aspetto un segnale

        //blocco segnali
        if (sigprocmask(SIG_BLOCK, &SigSet, NULL) == -1)
            errExit("mask fail");
        //cambio directory di lavoro
        char buf[FILE_PATH_MAX];
        //printf("%s", getcwd(buf, FILE_PATH_MAX));
        if (chdir(newDir) == -1)
            errExit("chdir failed");
        //output su terminale, si può usare printf? ricky dice di sì
        printf("\nCiao %s, ora inzio l'invio dei file contenuti in %s.", getenv("USER"), getcwd(buf, FILE_PATH_MAX));

        //controllo cartelle
        int n_file = readDir(newDir, FILE_SIZE_MAX, "sendme_");
        printf("\n%d file trovati: ", n_file);
        for (int i = 0; i < n_file; i++)
            printf("\n%d) %s", i, memAllPath[i]);
        fflush(stdout);


        //...

        if (sigprocmask(SIG_UNBLOCK, &SigSet, NULL) == -1)
            errExit("mask fail");
    }

    return 0;
}
