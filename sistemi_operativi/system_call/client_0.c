/// @file client.c
/// @brief Contiene l'implementazione del client.

#include "defines.h"

int semChilds;  //semaforo per coordinare i figli

void sigHandler(int sig) { //serve solo per interrompere la pause
    if (sig == SIGUSR1) {
        if (close(fifo1) == -1 || close(fifo2) == -1)
            errExit("Closing fifos FDs failed");

        free_shared_memory(shdmemBuffer);

        if (semctl(semChilds, 0, IPC_RMID) == -1)
            errExit("semctl IPC_RMID failed");

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
    if (msgq_k == -1)
        errExit("ftok msgq failed");
    key_t shdmem_k = ftok(getenv("HOME"), KEY_SHDMEM);
    if (shdmem_k == -1)
        errExit("ftok shdmem failed");

    key_t semShdmem_k = ftok(getenv("HOME"), KEY_SEM_SHDMEM);
    if (shdmem_k == -1)
        errExit("ftok semShdmem failed");

    key_t semMessages_k = ftok(getenv("HOME"), KEY_SEM_MESSAGES);
    if(shdmem_k==-1)
        errExit("ftok semShdmem failed");

    //prendo fifo1 creata da server
    sprintf(fifo1Path, "%s/%s", getenv("HOME"), PATH_FIFO1);  //concateno il nome del file alla cartella home
    while (access(fifo1Path, F_OK) == -1); //finchè la fifo non è stata creata aspetta

    fifo1 = open(fifo1Path, O_WRONLY);
    if (fifo1 == -1)
        errExit("open fifo1 failed");

    //prendo fifo2 creata da server
    sprintf(fifo2Path, "%s/%s", getenv("HOME"), PATH_FIFO2);
    while (access(fifo2Path, F_OK) == -1);

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

    semMessages = semget(semMessages_k, 3, S_IRUSR | S_IWUSR);
    if (semShdmemid == -1)
        errExit("semget failed");

    //creo semaforo per gestire figli
    semChilds = semget(IPC_PRIVATE, 1, S_IRUSR | S_IWUSR | IPC_CREAT);
    if(semChilds == -1)
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
        //printf("%s", getcwd(buf, FILE_PATH_MAX));
        if (chdir(newDir) == -1)
            errExit("chdir failed");

        if(getcwd(newDir, FILE_PATH_MAX)==NULL)
            errExit("getcwd failed");
        
        //output su terminale, si può usare printf? ricky dice di sì
        printf("\nCiao %s, ora inzio l'invio dei file contenuti in %s.", getenv("USER"), newDir);

        //controllo cartelle
        n_file = 0;  //readDir modificherà il valore della avriabile flobale n_file e riempirà l'array memallpath coi path dei files
        findFiles(newDir, FILE_SIZE_MAX, "sendme_");
        printf("\n%d file trovati: ", n_file);
        for (int i = 0; i < n_file; i++)
            printf("\n%d) %s", i, memAllPath[i]);
        fflush(stdout);

        //char n_fileString[4];
        //sprintf(n_fileString, "%d", n_file); //converto il numero di file in stringa da inviare sulla fifo
        if (write(fifo1, &n_file, sizeof(int)) == -1)  //scrivo sulla fifo1 il numero di file
            errExit("Write failed");

        printf("\nAttendo conferma ricezione da server");
        struct bareMessage message;
        read_from_shdmem(shdmemBuffer, &message, 1);
        if (strncmp("Conferma", message.part, strlen("Conferma")) == 0)
            printf("\nIl server conferma!");

        else
            printf("\nLa conferma ha un testo inaspettato: %s", message.part);
        fflush(stdout);

        //inizializzo il semaforo al numero di file = numero figli
        union semun arg;
        arg.val = n_file;
        if (semctl(semChilds, 0, SETVAL, arg) == -1)
            errExit("set semaphore fail\n");

        //creo n_file processi figli
        for (int child = 0; child < n_file; child++) {
            pid_t pid = fork();
            if (pid == -1)
                errExit("fork failed\n");

            //processo figlio
            if (pid == 0) {
                //apro il file
                int fdFile = open(memAllPath[child], O_RDONLY);
                if (fdFile == -1)
                    errExit("open file failed");

                //determino la dimensione
                struct stat statbuf;
                if (fstat(fdFile, &statbuf) == -1)
                    errExit("fstat file failed");

                off_t fileSize = (statbuf.st_size==0)? 0 : statbuf.st_size-1;  //col -1 tolgo il newline di ogni file però se il file è vuoto (size=0) non posso andare a -1

                //creo i 4 messaggi in cui il file deve essere inviato
                struct bareMessage messages[4];
                for (int i = 0; i < 4; i++) {
                    messages[i].pid = getpid();
                    strcpy(messages[i].path, memAllPath[child]);
                }

                off_t current = lseek(fdFile, 0, SEEK_SET);
                if (current == -1)
                    errExit("lseek failed");

                ///off_t charsNumber = fileSize / 4 + (fileSize % 4 !=0); //le divisioni tra positivi arrotondano a -inf, quindi visto che voglio arrotondare a +inf se il numero non è divisibile per 4 devo aggiungere 1 al risultato

                off_t quotient = fileSize / 4;
                off_t remainder = fileSize % 4;

                ///printf("\nfile %s, size %d, charsNumber %d", memAllPath[child], fileSize, charsNumber);

//                for (int i = 0, bLeft=fileSize, br=0; i < 4; i++) {
//
//                    //per evitare di leggere il new line che ha ogni file prendo il numero minore
//                    br = read(fdFile, messages[i].part, sizeof(char) * ((charsNumber < bLeft)? charsNumber: bLeft));
//                    if (br == -1)
//                        errExit("reading files failed");
//                    bLeft-=br;
//
//                    messages[i].part[br]='\0';  //appendo il terminatore alla fine di quello che ho copiato
//                }

                for(int i=0, br=0; i<4; i++){
                    br = read(fdFile, messages[i].part, quotient + ((i<remainder)?1 : 0)); //se ho resto 0 non aggiungo niente a nessun canale, se ho 1 aggiungo solo al primo
                    if(br == -1)
                        errExit("Reading files failed");

                    messages[i].part[br]='\0';
                }

                if(close(fdFile)==-1)  //chiudi
                    errExit("close file failed");

                printf("\nclient_%d aspetta\n", child + 1);
                fflush(stdout);
                semOp(semChilds, 0, -1, 1);
                semOp(semChilds, 0, 0, 1);
                printf("\nclient_%d parte\n", child + 1);
                fflush(stdout);

                // scrivo sulle ipcs
                write_fifo1(&messages[0]);
                write_fifo2(&messages[1]);
                msgQueueSend(messages[2], CLIENT_MTYPE);
                write_in_shdmem(shdmemBuffer, messages[3].path, messages[3].part);

                //chiudo i figli e stacco le robe
                return 0;
            }
        }

        //rispristino il ricevimento di INT e USR1 da parte di client_0
        if (sigprocmask(SIG_SETMASK, &SigSet, NULL) == -1)
            errExit("mask fail");
    }

    return 0;
}
