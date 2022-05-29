include_directories(.)

add_compile_options(-Wall -std=gnu99 -g)

add_executable(client_0
        client_0.c
        defines.c
        err_exit.c
        fifo.c
        semaphore.c
        shared_memory.c)

add_executable(server
        server.c
        defines.c
        err_exit.c
        fifo.c
        semaphore.c
        shared_memory.c)