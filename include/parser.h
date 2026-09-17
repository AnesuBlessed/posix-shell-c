#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>

/**
 * Command structure representing a single parsed command line
 * with its arguments and optional redirection targets.
 */
typedef struct {
    char **args;          /* NULL-terminated array of argument strings */
    int arg_count;        /* Number of arguments */
    char *stdout_file;    /* Target file for stdout redirection, or NULL */
    int stdout_append;    /* 1 if append (>>), 0 if overwrite (>) */
    char *stderr_file;    /* Target file for stderr redirection, or NULL */
    int stderr_append;    /* 1 if append (2>>), 0 if overwrite (2>) */
} Command;

/**
 * Parses raw input string into a Command struct.
 * Handles single quotes ('...'), double quotes ("..."), backslash escaping,
 * and standard I/O redirection operators (>, >>, 1>, 1>>, 2>, 2>>).
 */
Command* parse_command_line(const char *input);

/**
 * Frees memory associated with a Command structure.
 */
void free_command(Command *cmd);

#endif /* PARSER_H */
