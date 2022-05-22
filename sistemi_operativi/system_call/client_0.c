/// @file client.c
/// @brief Contiene l'implementazione del client.

#include "err_exit.h"
#include "defines.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "fifo.h"

char *newDir;
sigset_t SigSet;

char memAllPath[100][PATH_MAX]; // array per meorizzare i path dei file da inviare

//funzione per leggere tutti i file dentro ad una directory
int readDir(const char dirpath[], off_t maxSize) {

    int n_file = 0; // contatore numero di file che inziano con sendme_
    char nodePath[PATH_MAX];

    DIR *dirp = opendir(dirpath); //apro directory
    if (dirp == NULL)
        errExit("opendir failed");

    errno = 0;
    struct dirent *dentry;
    while ((dentry = readdir(dirp)) != NULL) { // scorro tutta la directory

        if (dentry->d_type == DT_REG && strncmp("sendme_", dentry->d_name, 7) == 0) { // caso base: ho letto un file che voglio
            sprintf(nodePath, "%s/%s", dirpath, dentry->d_name);  // ottengo path finale del file
            struct stat statbuf;
            if (stat(nodePath, &statbuf) == -1)
                errExit("stat failed");

            if(statbuf.st_size < maxSize) { //verifico che il file non pesi più di 4kb
                if(n_file>=100)  //massimo 100 file
                    exit(1);

                strcpy(memAllPath[n_file], nodePath);
                n_file++; // incremento contatore
            }

        } else if (dentry->d_type == DT_DIR && strcmp(dentry->d_name, ".")!=0 && strcmp(dentry->d_name, "..")!=0) {// se leggo una directory richiamo funzione
            sprintf(nodePath, "%s/%s", dirpath, dentry->d_name);
            n_file += readDir(nodePath, maxSize); // aggiungo i file trovati nelle sotto cartelle
        }

        errno = 0;
    }

    char errMsg[100] = "Error while reading directory ";
    strcat(errMsg, dirpath);
    if (errno != 0)
        errExit(errMsg);

    closedir(dirp);

    return n_file;
}

void sigHandler(int sig) { //serve solo per interrompere la pause
    if(sig==SIGUSR1)
        exit(0);
}

int main(int argc, char *argv[]) {

    //  ***** CONTROLLO CORRETEZZA INPUT: ./Client_0 <HOME>/myDir/ *****
    if (argc != 2) {
        char buffer[] = "\nUsage: ./client <HOME>/myDir"; //modifcare messaggio se neccessario
        if (write(STDOUT_FILENO, buffer, sizeof(buffer)) == -1)
            errExit("write on STDOUT failed");
        return 1;
    }

    newDir = argv[1];

    //lato client apertura di fifo, mssgqueue, shared memory...VA FATTA DAI CHILD?
    /*
    //creo path fifo1 e fifo2
    char *pathF1 = fifo1;
    char *pathF2 = fifo2;

    //creo le key
    key_t msgq_k = ftok(".", KEY_MSGQ);
    key_t shdmem_k = ftok(".", KEY_SHDMEM);

    //apro in scrittura le fifo
    int fifo1 = open(pathF1, O_WRONLY);
    if(fifo1 == -1)
      errExit("open fifo1 failed");

    int fifo2 = open(pathF2, O_WRONLY);
    if(fifo1 == -1)
      errExit("open fifo2 failed");

    //apro msg queue
    int msqid = msgget(msgq_k, S_IRUSR|S_IWUSR);
    if(msqid == -1)
      errExit("msgget failled");

    //attacco shared memory
    int shmidServer = alloc_shared_memory(shdmem_k);
    int *SHDMEMbuffer = get_shared_memory(shmidServer, 0);*/



    //  ***** SETTO SEGNALI *****
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
        //pause(); //aspetto un segnale

        //blocco segnali
        if (sigprocmask(SIG_BLOCK, &SigSet, NULL) == -1)
            errExit("mask fail");
        //cambio directory di lavoro
        char buf[PATH_MAX];
        //printf("%s", getcwd(buf, PATH_MAX));
        if (chdir(newDir) == -1)
            errExit("chdir failed");
        //output su terminale, si può usare printf? ricky dice di sì
        printf("\nCiao %s, ora inzio l'invio dei file contenuti in %s.", getenv("USER"), getcwd(buf, PATH_MAX));

        //controllo cartelle
        int n_file = readDir(newDir, 4096);
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
