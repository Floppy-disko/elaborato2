/// @file defines.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche del progetto.

#include "defines.h"

//funzione per leggere tutti i file dentro ad una directory
///@param dirpath contiene il path della directory in cui cercare
///@param maxSize contiene dimensione massima file da cercare
///@param match stringa che devono matchare i file per essere idonei al conteggio
int readDir(const char dirpath[], off_t maxSize, char *match) {

    int n_file = 0; // contatore numero di file che inziano con sendme_
    char nodePath[FILE_PATH_MAX];

    DIR *dirp = opendir(dirpath); //apro directory
    if (dirp == NULL)
        errExit("opendir failed");

    errno = 0;
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
            n_file += readDir(nodePath, maxSize, match); // aggiungo i file trovati nelle sotto cartelle
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
