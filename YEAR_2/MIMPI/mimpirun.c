/**
 * This file is for implementation of mimpirun program.
 * */

// Program mimpirun przyjmuje następujące argumenty linii poleceń:

// n - liczba kopii do uruchomienia (można założyć, że przekazana zostanie liczba naturalna z zakresu od 1 do
// włącznie)
// prog - ścieżka do pliku wykonywalnego (może się znajdować w PATH). W przypadku, gdy odpowiednie wywołanie exec się nie powiedzie (np. z powodu niepoprawnej ścieżki) należy zakończyć działanie mimpirun z niezerowym kodem wyjściowym.
// args - opcjonalnie i w dowolnej ilości argumenty do przekazania wszystkim uruchamianym programom

// Program mimpirun po kolei (następna czynność jest rozpoczynana po całkowitym zakończeniu poprzedniej):

// Przygotowuje środowisko (w zakresie implementującego jest zdecydowanie, co to znaczy).
// Uruchamia n kopii programu prog, każdą w osobnym procesie.
// Czeka na zakończenie wszystkich utworzonych procesów.
// Kończy działanie.

#include "mimpi_common.h"
#include "channel.h"
#include <unistd.h>
#include <stdio.h> 
#include <stdlib.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Too few arguments\n");
        return 1;
    }
    
    int n = atoi(argv[1]);
    char *prog = argv[2];

    if (n < 1) {
        fprintf(stderr, "Invalid number of processes\n");
        return 1;
    }

    /* set up environment */
    char const *world_count = "world_count";
    char const *world_rank = "world_rank";

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            int fd[2];
            ASSERT_SYS_OK(channel(fd));

            ASSERT_SYS_OK(dup2(fd[0], 20 + j + i * 16));
            ASSERT_SYS_OK(close(fd[0]));
            
            ASSERT_SYS_OK(dup2(fd[1], 300 + j +  i * 16));
            ASSERT_SYS_OK(close(fd[1]));
        }

        int fd[2];
        int fd2[2];
        int fd3[2];
        int fd4[2];
        ASSERT_SYS_OK(channel(fd));
        ASSERT_SYS_OK(channel(fd2));
        ASSERT_SYS_OK(channel(fd3));
        ASSERT_SYS_OK(channel(fd4));

        ASSERT_SYS_OK(dup2(fd[0], 560 + i * 8 + 1));
        ASSERT_SYS_OK(close(fd[0]));

        ASSERT_SYS_OK(dup2(fd[1], 560 + i * 8 + 2));
        ASSERT_SYS_OK(close(fd[1]));

        ASSERT_SYS_OK(dup2(fd2[0], 560 + i * 8 + 3));
        ASSERT_SYS_OK(close(fd2[0]));

        ASSERT_SYS_OK(dup2(fd2[1], 560 + i * 8 + 4));
        ASSERT_SYS_OK(close(fd2[1]));

        ASSERT_SYS_OK(dup2(fd3[0], 560 + i * 8 + 5));
        ASSERT_SYS_OK(close(fd3[0]));

        ASSERT_SYS_OK(dup2(fd3[1], 560 + i * 8 + 6));
        ASSERT_SYS_OK(close(fd3[1]));

        ASSERT_SYS_OK(dup2(fd4[0], 560 + i * 8 + 7));
        ASSERT_SYS_OK(close(fd4[0]));

        ASSERT_SYS_OK(dup2(fd4[1], 560 + i * 8 + 8));
        ASSERT_SYS_OK(close(fd4[1]));

        int fd5[2];
        int fd6[2];
        int fd7[2];
        int fd8[2];
        ASSERT_SYS_OK(channel(fd5));
        ASSERT_SYS_OK(channel(fd6));
        ASSERT_SYS_OK(channel(fd7));
        ASSERT_SYS_OK(channel(fd8));

        ASSERT_SYS_OK(dup2(fd5[0], 830 + i * 8 + 1));
        ASSERT_SYS_OK(close(fd5[0]));

        ASSERT_SYS_OK(dup2(fd5[1], 830 + i * 8 + 2));
        ASSERT_SYS_OK(close(fd5[1]));

        ASSERT_SYS_OK(dup2(fd6[0], 830 + i * 8 + 3));
        ASSERT_SYS_OK(close(fd6[0]));

        ASSERT_SYS_OK(dup2(fd6[1], 830 + i * 8 + 4));
        ASSERT_SYS_OK(close(fd6[1]));

        ASSERT_SYS_OK(dup2(fd7[0], 830 + i * 8 + 5));
        ASSERT_SYS_OK(close(fd7[0]));

        ASSERT_SYS_OK(dup2(fd7[1], 830 + i * 8 + 6));
        ASSERT_SYS_OK(close(fd7[1]));

        ASSERT_SYS_OK(dup2(fd8[0], 830 + i * 8 + 7));
        ASSERT_SYS_OK(close(fd8[0]));

        ASSERT_SYS_OK(dup2(fd8[1], 830 + i * 8 + 8));
        ASSERT_SYS_OK(close(fd8[1]));
    }   

    for (int i = 0; i < n; i++) {
        pid_t pid;
        ASSERT_SYS_OK(pid = fork());

        if (!pid) {
            char rank[sizeof(int) * 3];
            int ret = sprintf(rank, "%d", i);
            if (ret < 0 || ret > (int)sizeof(rank)) {
                fprintf(stderr, "Error while copying rank\n");
                return 1;
            }

            char n_copy[sizeof(int) * 3];
            ret = sprintf(n_copy, "%d", n);
            if (ret < 0 || ret > (int)sizeof(n_copy)) {
                fprintf(stderr, "Error while copying n\n");
                return 1;
            }

            ASSERT_SYS_OK(setenv(world_count, n_copy, 1));
            ASSERT_SYS_OK(setenv(world_rank, rank, 1));

            ASSERT_SYS_OK(execvp(prog, argv + 2));
        }
    }
 
    /* wait for all processes to finish */
    for (int i = 0; i < n; i++) 
        ASSERT_SYS_OK(wait(NULL));

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            ASSERT_SYS_OK(close(20 + j + i * 16));
            ASSERT_SYS_OK(close(300 + j + i * 16));
        }

        for (int j = 1; j <= 8; j++) {
            ASSERT_SYS_OK(close(560 + i * 8 + j));
            ASSERT_SYS_OK(close(830 + i * 8 + j));
        }
    }
     
    return 0;
}
