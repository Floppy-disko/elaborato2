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

#define KEY_MSGQ 'A'
#define KEY_SHDMEM 'B'

#define fifo1 "fifo1"
#define fifo2 "fifo2"

//array per in controllo della shared memory, dove è 0 è sopazio vuoto, dove 1 è occupato
int SHDMEMcontroll[50];


void sigHandler(int sig);