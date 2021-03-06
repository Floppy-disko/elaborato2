/// @file defines.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche del progetto.

#include "defines.h"

//funzione per leggere tutti i file dentro ad una directory
///@param dirpath contiene il path della directory in cui cercare
///@param maxSize contiene dimensione massima file da cercare
///@param match stringa che devono matchare i file per essere idonei al conteggio
int findFiles(const char dirpath[], off_t maxSize, char *match) {

    char nodePath[FILE_PATH_MAX];

    DIR *dirp = opendir(dirpath); //apro directory
    if (dirp == NULL)
        errExit("opendir failed");

    errno = 0;  //setto a 0 così se readdir restituisce NULL posso verificare se errno è ancora 0 o ha cambiato valore (readdir failed)
    struct dirent *dentry;
    while ((dentry = readdir(dirp)) != NULL) { // scorro tutta la directory

        if (dentry->d_type == DT_REG &&
            strncmp(match, dentry->d_name, strlen(match)) == 0) { // caso base: ho letto un file che voglio
            sprintf(nodePath, "%s/%s", dirpath, dentry->d_name);  // ottengo path finale del file
            struct stat statbuf;
            if (stat(nodePath, &statbuf) == -1)
                errExit("stat failed");

            if (statbuf.st_size < maxSize) { //verifico che il file non pesi più di 4kb
                strcpy(memAllPath[n_file], nodePath);
                n_file++; // incremento contatore

                if(n_file>=FILE_NUMBER_MAX)  //Se ho letto già 100 file mi fermo
                    return FILE_NUMBER_MAX;
            }

        } else if (dentry->d_type == DT_DIR && strcmp(dentry->d_name, ".") != 0 &&
                   strcmp(dentry->d_name, "..") != 0) {// se leggo una directory richiamo funzione
            sprintf(nodePath, "%s/%s", dirpath, dentry->d_name);
            findFiles(nodePath, maxSize, match); // cerco ricorsivamente nelle cartelle
        }

        errno = 0;
    }

    if (errno != 0)
        errExit("Error while reading directory");

    if(closedir(dirp)==-1)
        errExit("closedir failed");

    return n_file;
}

void write_fifo1(struct bareMessage *message) {
    semOp(semMessages, 0, -1, 1);
    if (write(fifo1, message, sizeof(struct bareMessage)) == -1)
        errExit("Write failed");
}

void write_fifo2(struct bareMessage *message) {
    semOp(semMessages, 1, -1, 1);
    if (write(fifo2, message, sizeof(struct bareMessage)) == -1)
        errExit("Write failed");
}

void msgQueueSend(struct bareMessage *message, long mtype) {

    struct clientMsg send;
    send.mtype = mtype;

    send.message = *message;

    semOp(semMessages, 2, -1, 1);  //per limitare il massimo numero di messaggi contemporanei a 50

    if (msgsnd(msqid, &send, sizeof(struct clientMsg) - sizeof(long), 0))
        errExit("msgsnd failed\n");
}

int msgQueueReceive(struct bareMessage *dest, long mtype, int wait) {

    struct clientMsg message;

    if (msgrcv(msqid, &message, sizeof(struct clientMsg) - sizeof(long), mtype, ((wait) ? 0 : IPC_NOWAIT)) == -1) {
        if (errno == ENOMSG)
            return -1;
        else
            errExit("msgrcv failed");
    }

    semOp(semMessages, 2, 1, 1); //informo che un messaggio è stato tolto dalla coda messaggi

    *dest = message.message;

    return 0;
}

///@param ptr_sh puntatore alla shared memory strutturata
///@param filePath path del file di cui invio un quarto
///@param text testo da scrivere nel bare message
void write_in_shdmem(struct shdmemStructure *ptr_sh, struct bareMessage *message) {

    semOp(semShdmemid, 2, -1, 1);  //vedo se ci sono spazi liberi, aspetto per forza che sia libero per scrivere
    semOp(semShdmemid, 0, -1, 1);  //prendo sezione critica

    ptr_sh->messages[ptr_sh->in] = *message;  //metto il messaggio nella prima cella libera del buffer circolare
    ptr_sh->in = (ptr_sh->in + 1) % MSG_NUMBER_MAX;  //quando ho raggiunto in=50 ricomincio d in=0

    semOp(semShdmemid, 0, 1, 1);  //libero sezione critica
    semOp(semShdmemid, 1, 1, 1);  //dico che c'è uno spazio pieno(full) in più
}

///@param wait a 1 se la lettura è bloccante, a 0 se la lettura non è bloccante
///@return -1 se non e bloccante e sarebbe stato bloccato, 0 se non viene bloccato
int read_from_shdmem(struct shdmemStructure *ptr_sh, struct bareMessage *dest, int wait) {

    if (semOp(semShdmemid, 1, -1, wait) == -1)  //vedo se c'è qualcosa da leggere, sennò ritorna -1
        return -1;

    semOp(semShdmemid, 0, -1, 1);

    //copio il messaggio in indice out nella destinazione dest
    *dest = ptr_sh->messages[ptr_sh->out];
    ptr_sh->out = (ptr_sh->out + 1) % MSG_NUMBER_MAX;

    semOp(semShdmemid, 0, 1, 1);
    semOp(semShdmemid, 2, 1, 1);
    return 0;
}
