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

        if (dentry->d_type == DT_REG && strncmp(match, dentry->d_name, strlen(match)) == 0) { // caso base: ho letto un file che voglio
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
            findFiles(nodePath, maxSize, match); // cerco ricorsivamente nelle cartelle
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

///@param ptr_sh puntatore alla shared memory strutturata
///@param filePath path del file di cui invio un quarto
///@param text testo da scrivere nel bare message
void write_in_shdmem(struct shdmemStructure *ptr_sh, char *filePath, char *text){
    struct bareMessage message = {
            getpid(),
            filePath,
            text
    };
    //TODO qui sarebbe da mettere un semaforo mutex per evitare che più di un processo acceda alla shared memory e uno per verificare che non sia già piena

    ptr_sh->messages[ptr_sh->in] = message;  //metto il messaggio nella prima cella libera del buffer circolare
    ptr_sh->in = (ptr_sh->in + 1) % MSG_NUMBER_MAX;  //quando ho raggiunto in=50 ricomincio d in=0
    //TODO qui sarebbe da aprire il semforo mutex e aumentare il semaforo della pienezza
}

///@param wait a 1 se la lettura è bloccante, a 0 se la lettura non è bloccante
struct bareMessage read_from_shdmem(struct shdmemStructure *ptr_sh, int wait){
    struct bareMessage message;
    //TODO sarebbe da mettere un semaforo mutex e uno per vedere che non sia vuota
    message = ptr_sh->messages[ptr_sh->out];  //prendo il messaggio della prima cella piena nel buffer circolare
    ptr_sh->out = (ptr_sh->out + 1) % MSG_NUMBER_MAX;
    //TODO aprire semaforo mutex e aumentare il semaforo della vuotezza
    return message;
}
