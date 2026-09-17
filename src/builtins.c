#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include "../include/builtins.h"
#include "../include/executor.h"

static const char *BUILTIN_NAMES[] = {
    "exit",
    "echo",
    "type",
    "pwd",
    "cd",
    "help",
    NULL
};

int is_builtin(const char *cmd) {
    if (!cmd) return 0;
    for (int i = 0; BUILTIN_NAMES[i] != NULL; ++i) {
        if (strcmp(cmd, BUILTIN_NAMES[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

static int builtin_exit(Command *cmd) {
    int code = 0;
    if (cmd->arg_count > 1) {
        code = atoi(cmd->args[1]);
    }
    free_command(cmd);
    exit(code);
}

static int builtin_echo(Command *cmd) {
    for (int i = 1; i < cmd->arg_count; ++i) {
        printf("%s", cmd->args[i]);
        if (i < cmd->arg_count - 1) {
            printf(" ");
        }
    }
    printf("\n");
    fflush(stdout);
    return 0;
}

static int builtin_pwd(void) {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
        fflush(stdout);
        return 0;
    } else {
        perror("pwd");
        return 1;
    }
}

static int builtin_cd(Command *cmd) {
    char cwd_buf[PATH_MAX];
    const char *target = NULL;

    if (cmd->arg_count <= 1 || strcmp(cmd->args[1], "~") == 0) {
        target = getenv("HOME");
        if (!target) {
            fprintf(stderr, "cd: HOME not set\n");
            return 1;
        }
    } else if (strcmp(cmd->args[1], "-") == 0) {
        target = getenv("OLDPWD");
        if (!target) {
            fprintf(stderr, "cd: OLDPWD not set\n");
            return 1;
        }
        printf("%s\n", target);
    } else {
        target = cmd->args[1];
    }

    /* Record current directory for OLDPWD */
    char *current_pwd = getcwd(cwd_buf, sizeof(cwd_buf));

    if (chdir(target) != 0) {
        fprintf(stderr, "cd: %s: No such file or directory\n", target);
        return 1;
    }

    if (current_pwd) {
        setenv("OLDPWD", current_pwd, 1);
    }

    if (getcwd(cwd_buf, sizeof(cwd_buf)) != NULL) {
        setenv("PWD", cwd_buf, 1);
    }

    return 0;
}

static int builtin_type(Command *cmd) {
    if (cmd->arg_count <= 1) {
        fprintf(stderr, "type: missing argument\n");
        return 1;
    }

    int overall_status = 0;
    for (int i = 1; i < cmd->arg_count; ++i) {
        const char *name = cmd->args[i];
        if (is_builtin(name)) {
            printf("%s is a shell builtin\n", name);
        } else {
            char *path = resolve_path(name);
            if (path) {
                printf("%s is %s\n", name, path);
                free(path);
            } else {
                printf("%s: not found\n", name);
                overall_status = 1;
            }
        }
    }
    fflush(stdout);
    return overall_status;
}

static int builtin_help(void) {
    printf("POSIX Shell in C (posix-shell)\n");
    printf("These shell commands are defined internally:\n");
    printf("  cd [dir]       Change the current working directory (~ for home, - for previous)\n");
    printf("  echo [arg ...] Output arguments separated by spaces\n");
    printf("  pwd            Print current working directory\n");
    printf("  type [name]    Display information about command type and path\n");
    printf("  help           Display this assistance information\n");
    printf("  exit [n]       Exit the shell with status code n (default: 0)\n");
    printf("\nSupports I/O redirection: >, >>, 2>, 2>> and single/double quotes.\n");
    fflush(stdout);
    return 0;
}

int execute_builtin(Command *cmd) {
    if (!cmd || cmd->arg_count == 0) return 0;
    const char *name = cmd->args[0];

    if (strcmp(name, "exit") == 0) return builtin_exit(cmd);
    if (strcmp(name, "echo") == 0) return builtin_echo(cmd);
    if (strcmp(name, "pwd") == 0) return builtin_pwd();
    if (strcmp(name, "cd") == 0) return builtin_cd(cmd);
    if (strcmp(name, "type") == 0) return builtin_type(cmd);
    if (strcmp(name, "help") == 0) return builtin_help();

    return 1;
}
