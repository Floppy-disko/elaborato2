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
#include <sys/sem.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#include<stdio.h>

#include "err_exit.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "fifo.h"

#define FILE_NUMBER_MAX 100
#define FILE_PATH_MAX 150
#define FILE_SIZE_MAX 4096
#define MSG_NUMBER_MAX 50

#define KEY_MSGQ 'A'
#define KEY_SHDMEM 'B'
#define KEY_SEM_SHDMEM 'C'
#define KEY_SEM_MESSAGES 'D'
#define PATH_FIFO1 "fifo1"
#define PATH_FIFO2 "fifo2"
#define CLIENT_MTYPE 1
#define SERVER_MTYPE 2

//costante per salvare valore tipo bareMessage
#define BAREM 4

struct bareMessage {
    pid_t pid;  //pid processo inviante
    char path[FILE_PATH_MAX];  //path file
    char part[1024];      //quarto del file di testo
};

//struct per msg queue
struct mymsg{
  long mtype;
  struct bareMessage message;
};

//struttura che rappresenta l'intera shared memory
struct shdmemStructure{  //utilizza array di 50 int come vettore di supporto
    int in;
    int out;  //in e out sono gli stessi del produttore-consumatore su un buffer circolare in sistemi operativi
    struct bareMessage messages[MSG_NUMBER_MAX];
};

//definizione variabili per ipc, fifo e semafori
int fifo1;
char fifo1Path[FILE_PATH_MAX];
int fifo2;
char fifo2Path[FILE_PATH_MAX];
int msqid;
int shdmemid;
struct shdmemStructure *shdmemBuffer;
int semShdmemid;  //3 semafori per gestire la msgq come memoria circolare
int semMessages; //3 semafori per fare in modo di avere max 50 messaggi su fifo1, fifo2 e msgq

int n_file; //variabile modificata da findFiles

char memAllPath[FILE_NUMBER_MAX][FILE_PATH_MAX]; // array per memorizzare i path dei file da inviare/ ricevuti

int findFiles(const char dirpath[], off_t maxSize, char *match); //se vuoi che invii path assoluti passa un path assoluto come valore

void write_fifo1(struct bareMessage *message);
void write_fifo2(struct bareMessage *message);

void msgQueueSend(struct bareMessage message, long mtype);
int msgQueueReceive(struct bareMessage *dest, long mtype, int wait);

//scrivo un bareMessage nella shared memory
void write_in_shdmem(struct shdmemStructure *ptr_sh, char *filePath, char *text);

int read_from_shdmem(struct shdmemStructure *ptr_sh, struct bareMessage *dest, int wait);
