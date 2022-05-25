/// @file client.c
/// @brief Contiene l'implementazione del client.

#include "defines.h"

void sigHandler(int sig) { //serve solo per interrompere la pause
    if(sig==SIGUSR1) {
        if(close(fifo1)==-1 || close(fifo2)==-1)
            errExit("Closing fifos FDs failed");

        free_shared_memory(shdmemBuffer);
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

    //  ***** SETTO SEGNALI *****
    sigset_t SigSet;
    if (sigfillset(&SigSet) == -1)
        errExit("filling mySet failed");
    if (sigdelset(&SigSet, SIGINT) == -1)
        errExit("deleting mySet failed");
    if (sigdelset(&SigSet, SIGUSR1) == -1)
        errExit("deleting mySet failed");
    if (sigprocmask(SIG_SETMASK, &SigSet, NULL) == -1)
        errExit("mask fail");

    char newDir[FILE_PATH_MAX];
    strcpy(newDir, argv[1]);

    //lato client apertura di fifo, mssgqueue, shared memory...VA FATTA DAI CHILD?

    key_t msgq_k = ftok(getenv("HOME"), KEY_MSGQ);
    if(msgq_k==-1)
        errExit("ftok msgq failed");
    key_t shdmem_k = ftok(getenv("HOME"), KEY_SHDMEM);
    if(shdmem_k==-1)
        errExit("ftok shdmem failed");

    key_t semShdmem_k = ftok(getenv("HOME"), KEY_SEM_SHDMEM);
    if(shdmem_k==-1)
        errExit("ftok semShdmem failed");

    //prendo fifo1 creata da server
    sprintf(fifo1Path, "%s/%s", getenv("HOME"), PATH_FIFO1);  //concateno il nome del file alla cartella home
    while(access(fifo1Path, F_OK)==-1); //finchè la fifo non è stata creata aspetta

    fifo1 = open(fifo1Path, O_WRONLY);
    if (fifo1 == -1)
        errExit("open fifo1 failed");

    //prendo fifo2 creata da server
    sprintf(fifo2Path, "%s/%s", getenv("HOME"), PATH_FIFO2);
    while(access(fifo2Path, F_OK)==-1);

    fifo2 = open(fifo2Path, O_WRONLY);
    if (fifo2 == -1)
        errExit("open fifo2 failed");

    //prendo msg queue creata da server
    msqid = msgget(msgq_k, S_IRUSR | S_IWUSR);
    if (msqid == -1)
        errExit("msgget failled");

    //ottengo shd memory creata da server
    shdmemid = alloc_shared_memory(shdmem_k, 0);
    shdmemBuffer = get_shared_memory(shdmemid, 0);

    semShdmemid = semget(semShdmem_k, 3, S_IRUSR | S_IWUSR);
    if (semShdmemid == -1)
        errExit("semget failed");

    //setto gli handler
    if (signal(SIGINT, sigHandler) == SIG_ERR)
        errExit("change signal handler failed");
    if (signal(SIGUSR1, sigHandler) == SIG_ERR)
        errExit("change signal handler failed");

    //loop con il codice vero e proprio del main
    while (1) {
        pause(); //aspetto un segnale

        //blocco anche INT e USR1
        sigset_t SigSet2;
        if (sigfillset(&SigSet2) == -1)
            errExit("filling mySet fail");
        if (sigprocmask(SIG_SETMASK, &SigSet2, NULL) == -1)
            errExit("mask fail");
        //cambio directory di lavoro
        char buf[FILE_PATH_MAX];
        //printf("%s", getcwd(buf, FILE_PATH_MAX));
        if (chdir(newDir) == -1)
            errExit("chdir failed");
        //output su terminale, si può usare printf? ricky dice di sì
        printf("\nCiao %s, ora inzio l'invio dei file contenuti in %s.", getenv("USER"), getcwd(buf, FILE_PATH_MAX));

        //controllo cartelle
        n_file = 0;  //readDir modificherà il valore della avriabile flobale n_file e riempirà l'array memallpath coi path dei files
        findFiles(newDir, FILE_SIZE_MAX, "sendme_");
        printf("\n%d file trovati: ", n_file);
        for (int i = 0; i < n_file; i++)
            printf("\n%d) %s", i, memAllPath[i]);
        fflush(stdout);

        char n_fileString[4];
        sprintf(n_fileString, "%d", n_file); //converto il numero di file in stringa da inviare sulla fifo
        if(write(fifo1, n_fileString, strlen(n_fileString)+1)==-1)  //scrivo sulla fifo1 il numero di file
            errExit("Write failed");

        printf("\nAttendo conferma ricezione da server");
        struct bareMessage message = read_from_shdmem(shdmemBuffer, 1);
        if(strncmp("Conferma", message.part, strlen("Conferma")-1)==0)
            printf("\nIl server conferma!");

        else
            printf("\nLa conferma ha un testo inaspettato: %s", message.part);
        fflush(stdout);
        //...

      //creo n_file processi figli
      for(int child=0; child<n_file; child++){
        pid_t pid = fork();
        if(pid == -1)
          errExit("fork failed\n");

        //processo figlio
        if(pid == 0){
          //apro il file
          int fdFile = open(memAllPath[child], O_RDONLY);
          if(fdFile == -1)
            errExit("open file failed");

          //determino la dimensione
          struct stat statbuf;
          fstat(fd, statbuf);
          off_t sizeFile = statbuf.st_size;

          char S1[sizeFile+1] = "";
          char S2[sizeFile+1] = "";
          char S3[sizeFile+1] = "";
          char S4[sizeFile+1] = "";

          lseek(fd, 0, SEEK_SET);
          
        }
      }

        //rispristino il ricevimento di INT e USR1
        if (sigprocmask(SIG_SETMASK, &SigSet, NULL) == -1)
            errExit("mask fail");
    }

    return 0;
}
