/// @file client.c
/// @brief Contiene l'implementazione del client.

#include "err_exit.h"
#include "defines.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "fifo.h"

#include <stdio.h>

char *newDir;
sigset_t SigSet;

struct str{
  char Path[150];
};
struct str memAllPath [100]; // array per meorizzare i path dei file da inviare

//funzione per leggere tutti i file dentro ad una directory
int readDir(const char dirpath[]){
  
  int n_file = 0; // contatore numero di file che inziano con sendme_
  char dest[strlen(dirpath)];

  DIR *dirp = opendir(dirpath); //apro directory
  if(dirp == NULL)
    errExit("opendir failed");
  
  struct dirent *dentry;
  while((dentry = readdir(dirp)) != NULL){ // scorro tutta la directory
    strcpy(dest, dirpath);
    
    if(dentry->d_type == DT_REG){ // caso base: ho letto un file
      if(strncmp("sendme_", dentry->d_name, 7) == 0){ // valuto se è quello che voglio
        strcat(dest, dentry->d_name);  // ottengo path finale
        strcpy(memAllPath[n_file].Path, dest);
        n_file++; // incremento contatore
      }
      
    }else if(dentry->d_type == DT_DIR){// se leggo una directory richiamo funzione
      strcat(dest, dentry->d_name); 
      n_file += readDir(dentry->d_name); // aggiungo i file trovati nelle sotto cartelle
      }
  }

  closedir(dirp);
  
  return n_file;
}

void sigHandler(int sig) {
    if(sig == SIGUSR1)
      exit(0);
    if(sig == SIGINT){
      
      //blocco segnali
      if(sigprocmask(SIG_BLOCK, &SigSet, NULL) == -1)
        errExit("mask fail");
      //cambio directory di lavoro
      if(chdir(newDir) == -1)
        errExit("chdir failed");
      //output su terminale, si può usare printf? ricky dice di sì
      printf("Ciao %s, ora inzio l'invio dei file contenuti in %s.\n", getenv("USER"), getenv("PWD"));

     //controllo cartelle
      int n_file = readDir(newDir);
      
      
      if(sigprocmask(SIG_UNBLOCK, &SigSet, NULL) == -1)
        errExit("mask fail");
    }
}

int main(int argc, char * argv[]) {

  //  ***** CONTROLLO CORRETEZZA INPUT: ./Client_0 <HOME>/myDir/ *****
  if (argc != 2) {
        char buffer[] = "Usage: ./client <HOME>myDir\n"; //modifcare messaggio se neccessario
        if(write(STDOUT_FILENO, buffer, sizeof(buffer)) == -1)
          errExit("write on STDOUT failed");
        return 1;
    }
  newDir = argv[1];

  
  //lato client apertura di fifo, mssgqueue, shared memory...VA FATTA DAI CHILD?
  /* 
  //creo path fifo1 e fifo2
  char *pathF1 = fifo1;
  char *pathF2 = fifo2;

  //creo le key 
  key_t msgq_k = ftok(".", KEY_MSGQ);
  key_t shdmem_k = ftok(".", KEY_SHDMEM);

  //apro in scrittura le fifo
  int fifo1 = open(pathF1, O_WRONLY);
  if(fifo1 == -1)
    errExit("open fifo1 failed");

  int fifo2 = open(pathF2, O_WRONLY);
  if(fifo1 == -1)
    errExit("open fifo2 failed");

  //apro msg queue
  int msqid = msgget(msgq_k, S_IRUSR|S_IWUSR);
  if(msqid == -1)
    errExit("msgget failled");

  //attacco shared memory
  int shmidServer = alloc_shared_memory(shdmem_k);
  int *SHDMEMbuffer = get_shared_memory(shmidServer, 0);*/


  
  //  ***** SETTO SEGNALI *****
  if(sigfillset(&SigSet) == -1)
      errExit("filling mySet fail");
  if(sigdelset(&SigSet, SIGINT) == -1)
    errExit("deleting mySet fail");
  if(sigdelset(&SigSet, SIGUSR1) == -1)
    errExit("deleting mySet fail");
  if(sigprocmask(SIG_SETMASK, &SigSet, NULL) == -1)
    errExit("mask fail");
  //setto gli handler
  if(signal(SIGINT, sigHandler) == SIG_ERR)
    errExit("change signal handler failed");
  if(signal(SIGUSR1, sigHandler) == SIG_ERR)
    errExit("change signal handler failed");

  //loop di attesa di SIGINT
  while(1){
    sleep(10);
  }

    return 0;
}
