#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "../include/executor.h"
#include "../include/builtins.h"

char* resolve_path(const char *command) {
    if (!command || *command == '\0') return NULL;

    /* If command contains a slash, treat it as a direct or relative path */
    if (strchr(command, '/') != NULL) {
        struct stat st;
        if (stat(command, &st) == 0 && S_ISREG(st.st_mode) && (access(command, X_OK) == 0)) {
            return strdup(command);
        }
        return NULL;
    }

    const char *path_env = getenv("PATH");
    if (!path_env) return NULL;

    char *path_copy = strdup(path_env);
    if (!path_copy) return NULL;

    char *saveptr = NULL;
    char *dir = strtok_r(path_copy, ":", &saveptr);
    char full_path[1024];

    while (dir != NULL) {
        snprintf(full_path, sizeof(full_path), "%s/%s", dir, command);
        struct stat st;
        if (stat(full_path, &st) == 0 && S_ISREG(st.st_mode) && (access(full_path, X_OK) == 0)) {
            free(path_copy);
            return strdup(full_path);
        }
        dir = strtok_r(NULL, ":", &saveptr);
    }

    free(path_copy);
    return NULL;
}

static int apply_redirections(Command *cmd) {
    if (cmd->stdout_file) {
        int flags = O_WRONLY | O_CREAT | (cmd->stdout_append ? O_APPEND : O_TRUNC);
        int fd = open(cmd->stdout_file, flags, 0644);
        if (fd < 0) {
            perror(cmd->stdout_file);
            return -1;
        }
        if (dup2(fd, STDOUT_FILENO) < 0) {
            perror("dup2 stdout");
            close(fd);
            return -1;
        }
        close(fd);
    }

    if (cmd->stderr_file) {
        int flags = O_WRONLY | O_CREAT | (cmd->stderr_append ? O_APPEND : O_TRUNC);
        int fd = open(cmd->stderr_file, flags, 0644);
        if (fd < 0) {
            perror(cmd->stderr_file);
            return -1;
        }
        if (dup2(fd, STDERR_FILENO) < 0) {
            perror("dup2 stderr");
            close(fd);
            return -1;
        }
        close(fd);
    }

    return 0;
}

int execute_command(Command *cmd) {
    if (!cmd || cmd->arg_count == 0) return 0;

    const char *cmd_name = cmd->args[0];

    /* Handle internal built-ins */
    if (is_builtin(cmd_name)) {
        int saved_stdout = -1;
        int saved_stderr = -1;

        if (cmd->stdout_file) {
            saved_stdout = dup(STDOUT_FILENO);
        }
        if (cmd->stderr_file) {
            saved_stderr = dup(STDERR_FILENO);
        }

        if (apply_redirections(cmd) < 0) {
            if (saved_stdout >= 0) { dup2(saved_stdout, STDOUT_FILENO); close(saved_stdout); }
            if (saved_stderr >= 0) { dup2(saved_stderr, STDERR_FILENO); close(saved_stderr); }
            return 1;
        }

        int status = execute_builtin(cmd);

        if (saved_stdout >= 0) {
            fflush(stdout);
            dup2(saved_stdout, STDOUT_FILENO);
            close(saved_stdout);
        }
        if (saved_stderr >= 0) {
            fflush(stderr);
            dup2(saved_stderr, STDERR_FILENO);
            close(saved_stderr);
        }

        return status;
    }

    /* Handle external executables */
    char *executable_path = resolve_path(cmd_name);
    if (!executable_path) {
        fprintf(stderr, "%s: command not found\n", cmd_name);
        return 127;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        free(executable_path);
        return 1;
    }

    if (pid == 0) {
        /* Child Process */
        if (apply_redirections(cmd) < 0) {
            _exit(EXIT_FAILURE);
        }

        execv(executable_path, cmd->args);
        perror(cmd_name);
        _exit(127);
    } else {
        /* Parent Process */
        free(executable_path);
        int status = 0;
        if (waitpid(pid, &status, 0) < 0) {
            perror("waitpid");
            return 1;
        }

        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            return 128 + WTERMSIG(status);
        }
        return 1;
    }
}
