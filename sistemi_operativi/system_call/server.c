/// @file sender_manager.c
/// @brief Contiene l'implementazione del sender_manager.

#include "defines.h"

int fifo1_block=-1;  //finchè rimane a -1 il sigHandler sa che non l'ho ancora aperto e quindi non prova a chiuderlo

//ristorna 0 per non ricevute tutte e 1 per ricevute tutte
int allPartsReceived(int indexes[]){
    for(int i=0; i<4; i++)
        if(indexes[i]<n_file)  //se anche solo un ipc non ha ricevuto tutte le parti ritorna 0
            return 0;

    return 1;
}

void try_fifo(int fifo, struct bareMessage messages[], int *index){
    //finchè non ho riempito tutte le parti ho la read non ritorna -1 con errno==EAGAIN
    while(*index<n_file) {

        int n=0; ///
        printf("\nProvo a leggere da fifo1 per la %d volta", n);
        fflush(stdout);
        //read ritorna -1 con errno EAGAIN se non si blocca data la flag O_NONBLOCK
        int br = read(fifo, &messages[*index], sizeof(struct bareMessage));
        if (br == -1) {
            if (errno == EAGAIN)  //non ho errori è solo vuota
                return;

            else
                errExit("Nonblocking read fifo1 failed");
        }

        printf("\nHo letto da fifo1");
        fflush(stdout);
        semOp(semMessages, 0, 1, 1);
        (*index)++;  //se ho letto un messaggio incremento index di 1
        n++; ///
    }

}

void try_msgq(struct bareMessage messages[], int *index){
    while(*index<n_file) {

        if (msgQueueReceive(&messages[*index], CLIENT_MTYPE, 0) == 0)
            (*index)++;
    }
}

void try_shdmem(struct bareMessage messages[], int *index){
    while(*index<n_file) {

        if (read_from_shdmem(shdmemBuffer, &messages[*index], 0) == 0)
            (*index)++;
    }
}

int createOutputFile(char *path){
    char newPath[FILE_PATH_MAX];
    int nameLen = strlen(path)-strlen(".txt"); //lunghezza nome file senza l'estensione .txt
    strncpy(newPath, path, nameLen);
    newPath[nameLen]='\0';  //la strncpy non aggiunge il terminatore se non ci arriva, ce lo devo mettere io
    strcat(newPath, "_out.txt");
    printf("\nnewPath: %s, nameLen: %d", newPath, nameLen);
    int fd = open(newPath, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR); //creo il file
    if(fd==-1)
        errExit("Creation of output file failed");

    return fd;
}

int searchPartIndex(struct bareMessage messages[], int pid){
    for(int i=0; i<n_file; i++){
        if(messages[i].pid == pid)
            return i;
    }

    return -1; //non dvrei arrivarci mai se il programma funziona
}

void sigHandler(int sig){
    if(sig==SIGINT){     //chiudo ed elimino le ipc
        if(close(fifo1)==-1 || close(fifo2)==-1)
            errExit("Closing nonblocking fifos failed");

        if(fifo1_block!=-1 && close(fifo1_block)==-1)
            errExit("Closing fifo1_block failed");

        if(unlink(fifo1Path)==-1 || unlink(fifo2Path)==-1)
            errExit("Unlinking fifos failed");

        if (msgctl(msqid, IPC_RMID, NULL) == -1)
            errExit("msgctl failed");

        free_shared_memory(shdmemBuffer);
        remove_shared_memory(shdmemid);

        if (semctl(semMessages, 0, IPC_RMID) == -1)
            errExit("semctl IPC_RMID failed");

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

    key_t semMessages_k = ftok(getenv("HOME"), KEY_SEM_MESSAGES);
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

    semMessages = semget(semMessages_k, 3, S_IRUSR | S_IWUSR | IPC_CREAT);
    if (semShdmemid == -1)
        errExit("semget failed");

    unsigned short semMessagesInitVal[] = {MSG_NUMBER_MAX, MSG_NUMBER_MAX, MSG_NUMBER_MAX};  //max 50 messaggi per ogni ipc
    arg.array = semMessagesInitVal;

    if (semctl(semMessages, 0, SETALL, arg))
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
    fifo1 = open(fifo1Path, O_RDONLY | O_NONBLOCK);
    if (fifo1 == -1)
        errExit("open fifo1 in nonblocking mode failed");
    fifo2 = open(fifo2Path, O_RDONLY | O_NONBLOCK);
    if (fifo2 == -1)
        errExit("open fifo2 failed");

    if (signal(SIGINT, sigHandler) == SIG_ERR)
        errExit("change signal handler failed");

    fifo1_block = open(fifo1Path, O_RDONLY);  //mi serve solo per la lettura del numero di file bloccante
    if (fifo1_block == -1)
        errExit("open fifo1 in blocking mode failed");

    while(1) {
        printf("\nLeggo il numero di n_file da fifo1");
        int br;

        do {  //provo a leggere finchè non trovo qualcosa di diverso dal carattere terminatore (raro caso in cui la write end viene chiusa)
            br = read(fifo1_block, &n_file, sizeof(int));
            if (br == -1)
                errExit("Read failed");
        } while(br==0);

        printf("\nHo letto: %d, invio conferma a client_0\n", n_file);
        fflush(stdout);
        write_in_shdmem(shdmemBuffer, "", "Conferma ricevimento n_file");

        printf("\nMi metto in ascolto delle parti di file");

        struct bareMessage messages[4][n_file];
        int indexes[4]={0};  //inizializzati a 0 arriveranno fino al valore di n_file

        while(!allPartsReceived(indexes)){
            try_fifo(fifo1, messages[0], &indexes[0]);
            try_fifo(fifo2, messages[1], &indexes[1]);
            try_msgq(messages[2], &indexes[2]);
            try_shdmem(messages[3], &indexes[3]);
        }

        for(int i=0; i<n_file; i++) {
            printf("\nfile %s:\n", messages[0][i].path);
            for (int j = 0; j < 4; j++) {
                printf("%s,", messages[j][i].part);
            }
        }

        printf("\nCreo i file di output");

        char *channelNames[4] = {"FIFO1", "FIFO2", "MsgQueue", "ShdMem"};  //mi salvo il nome di tutti i canali da metter nei file di output
        for(int i=0; i<n_file; i++){
            int fd = createOutputFile(messages[0][i].path); //0 perché ordino i file in base all'ordine con cui ho letto la loro parte 1 da fifo1
            for(int j=0; j<4; j++){
                char buf[FILE_PATH_MAX*2];

                sprintf(buf, "[Parte %d, del file %s, spedita dal processo %d tramite %s]\n", j+1, messages[0][i].path, messages[0][i].pid, channelNames[j]);
                if(write(fd, buf, strlen(buf)) == -1)
                    errExit("Write on output file failed");

                int partIndex = searchPartIndex(messages[j], messages[0][i].pid);  //cerco in base al pid dell'iesimo messaggio su fifo
                int br = write(fd, messages[j][partIndex].part, strlen(messages[j][partIndex].part));
                if(br == -1)
                    errExit("write on output file failed");

                if(write(fd, (br==0)? "\n" : "\n\n", 2) == -1)  //dopo ogni parte va una newline
                    errExit("write on output file failed");
            }

            close(fd);
        }

        struct serverMsg confirm = {SERVER_MTYPE};
        printf("\nInvio conferma a client di aver fatto tutto");
        if (msgsnd(msqid, &confirm, sizeof(struct serverMsg) - sizeof(long), 0))
            errExit("msgsnd failed\n");

    }

    return 0;
}

