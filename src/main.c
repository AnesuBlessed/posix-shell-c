#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "../include/parser.h"
#include "../include/executor.h"

static void sigint_handler(int sig) {
    (void)sig;
    /* Print a clean newline and re-display prompt */
    write(STDOUT_FILENO, "\n$ ", 3);
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    /* Install SIGINT (Ctrl+C) handler for interactive sessions */
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa, NULL);

    int is_interactive = isatty(STDIN_FILENO);

    char *line = NULL;
    size_t line_cap = 0;
    int last_exit_code = 0;

    while (1) {
        if (is_interactive) {
            printf("$ ");
            fflush(stdout);
        }

        ssize_t bytes_read = getline(&line, &line_cap, stdin);
        if (bytes_read == -1) {
            /* EOF received (Ctrl+D) */
            if (is_interactive) {
                printf("\n");
            }
            break;
        }

        /* Remove trailing newline / carriage returns */
        while (bytes_read > 0 && (line[bytes_read - 1] == '\n' || line[bytes_read - 1] == '\r')) {
            line[--bytes_read] = '\0';
        }

        /* Parse and execute */
        Command *cmd = parse_command_line(line);
        if (cmd) {
            last_exit_code = execute_command(cmd);
            free_command(cmd);
        }
    }

    if (line) {
        free(line);
    }

    return last_exit_code;
}
