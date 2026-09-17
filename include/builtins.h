#ifndef BUILTINS_H
#define BUILTINS_H

#include "parser.h"

/**
 * Checks whether the specified command name is an internal shell builtin.
 * Returns 1 if true, 0 otherwise.
 */
int is_builtin(const char *cmd);

/**
 * Executes a shell builtin command (exit, echo, type, pwd, cd, help).
 * Returns the command's exit code (typically 0 on success, non-zero on error).
 */
int execute_builtin(Command *cmd);

#endif /* BUILTINS_H */
