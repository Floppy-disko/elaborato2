/// @file client.c
/// @brief Contiene l'implementazione del client.

#include "defines.h"
#include<unistd.h>//
#include<stdio.h>//

int main(int argc, char * argv[]) {

    pid_t p = fork();
    if(p==-1)
        errExit("fork failed");
    if(p==0)
        printf("\nFiglio");
    else
        printf("\nPadre");


    return 0;
}
