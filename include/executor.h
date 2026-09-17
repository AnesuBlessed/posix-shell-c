#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "parser.h"

/**
 * Searches directories in $PATH to find executable binary.
 * Returns a newly heap-allocated string with absolute path if found, or NULL.
 * Caller is responsible for freeing the returned string.
 */
char* resolve_path(const char *command);

/**
 * Executes a command (either builtin or external binary).
 * Handles file descriptor redirection for both builtins and external processes.
 * Returns the process exit status.
 */
int execute_command(Command *cmd);

#endif /* EXECUTOR_H */
