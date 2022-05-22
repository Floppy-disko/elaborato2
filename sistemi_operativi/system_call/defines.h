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

#define FILE_PATH_MAX 100

#define KEY_MSGQ 'A'
#define KEY_SHDMEM 'B'
#define PATH_FIFO1 "fifo1"
#define PATH_FIFO2 "fifo2"
#define MSG_MAX

//array per in controllo della shared memory, dove è 0 è sopazio vuoto, dove 1 è occupato
int shdmControll[50];

struct bareMessage {
    pid_t pid;  //pid processo inviante
    char path[FILE_PATH_MAX];  //path file
    char part[1024];      //quarto del file di testo
};