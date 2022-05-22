/// @file defines.h
/// @brief Contiene la definizioni di variabili
///         e funzioni specifiche del progetto.

#pragma once
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include<stdio.h>

#include "err_exit.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "fifo.h"

#define FILE_PATH_MAX 100
#define FILE_SIZE_MAX 4096

#define KEY_MSGQ 'A'
#define KEY_SHDMEM 'B'
#define PATH_FIFO1 "fifo1"
#define PATH_FIFO2 "fifo2"
#define MSG_MAX

//array per in controllo della shared memory, dove è 0 è sopazio vuoto, dove 1 è occupato
int shdmControl[50];

struct bareMessage {
    pid_t pid;  //pid processo inviante
    char path[FILE_PATH_MAX];  //path file
    char part[1024];      //quarto del file di testo
};

//definizione variabili ipc e fifo
int fifo1;
char fifo1Path[FILE_PATH_MAX];
int fifo2;
char fifo2Path[FILE_PATH_MAX];
int msqid;
int shdmemid;
void *shdememBuffer;

char memAllPath[100][FILE_PATH_MAX]; // array per memorizzare i path dei file da inviare/ ricevuti

int readDir(const char dirpath[], off_t maxSize, char *match);