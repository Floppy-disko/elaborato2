/// @file shared_memory.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione della MEMORIA CONDIVISA.

#include "err_exit.h"
#include "shared_memory.h"

#include <sys/shm.h>
#include <sys/stat.h>



 int alloc_shared_memory(key_t shmKey, size_t size) {
   
    int shmid = shmget(shmKey, size, IPC_CREAT|S_IWUSR|S_IRUSR);
    if(shmid== -1)
      errExit("shmget failled");

    return shmid;
}

void *get_shared_memory(int shmid, int shmflg) {
    // attach the shared memory
    void *shmadr = shmat(shmid, NULL, shmflg);

    if(shmadr == (void*)-1)
      errExit("shmat failled");
  
    return shmadr;
}

void free_shared_memory(void *ptr_sh) {
    // detach the shared memory segments
    if(shmdt(ptr_sh) == -1)
      errExit("shmdt failled");
}

void remove_shared_memory(int shmid) {
    // delete the shared memory segment
    if(shmctl(shmid, IPC_RMID, NULL) == -1)
      errExit("shmctl remove failled");
}
