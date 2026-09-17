#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/parser.h"

#define INITIAL_TOKEN_BUF_SIZE 128
#define INITIAL_ARGS_CAPACITY 16

static void append_char(char **buf, size_t *len, size_t *cap, char c) {
    if (*len + 1 >= *cap) {
        *cap *= 2;
        char *new_buf = realloc(*buf, *cap);
        if (!new_buf) {
            perror("realloc token");
            exit(EXIT_FAILURE);
        }
        *buf = new_buf;
    }
    (*buf)[(*len)++] = c;
    (*buf)[*len] = '\0';
}

static char** tokenize(const char *input, int *count) {
    size_t args_cap = INITIAL_ARGS_CAPACITY;
    int args_len = 0;
    char **tokens = malloc(args_cap * sizeof(char*));
    if (!tokens) {
        perror("malloc tokens");
        exit(EXIT_FAILURE);
    }

    size_t token_cap = INITIAL_TOKEN_BUF_SIZE;
    size_t token_len = 0;
    char *token_buf = malloc(token_cap);
    if (!token_buf) {
        perror("malloc token_buf");
        exit(EXIT_FAILURE);
    }
    token_buf[0] = '\0';

    int in_single_quote = 0;
    int in_double_quote = 0;
    int has_content = 0;

    for (size_t i = 0; input[i] != '\0'; ++i) {
        char c = input[i];

        if (in_single_quote) {
            if (c == '\'') {
                in_single_quote = 0;
            } else {
                append_char(&token_buf, &token_len, &token_cap, c);
                has_content = 1;
            }
        } else if (in_double_quote) {
            if (c == '"') {
                in_double_quote = 0;
            } else if (c == '\\') {
                char next = input[i + 1];
                if (next == '\\' || next == '$' || next == '"' || next == '\n') {
                    append_char(&token_buf, &token_len, &token_cap, next);
                    i++;
                } else {
                    append_char(&token_buf, &token_len, &token_cap, '\\');
                }
                has_content = 1;
            } else {
                append_char(&token_buf, &token_len, &token_cap, c);
                has_content = 1;
            }
        } else {
            /* Outside quotes */
            if (c == '\'') {
                in_single_quote = 1;
                has_content = 1;
            } else if (c == '"') {
                in_double_quote = 1;
                has_content = 1;
            } else if (c == '\\') {
                char next = input[i + 1];
                if (next != '\0') {
                    append_char(&token_buf, &token_len, &token_cap, next);
                    i++;
                    has_content = 1;
                }
            } else if (isspace((unsigned char)c)) {
                if (has_content) {
                    /* Commit token */
                    if ((size_t)args_len + 1 >= args_cap) {
                        args_cap *= 2;
                        tokens = realloc(tokens, args_cap * sizeof(char*));
                        if (!tokens) { perror("realloc tokens"); exit(EXIT_FAILURE); }
                    }
                    tokens[args_len++] = strdup(token_buf);
                    token_len = 0;
                    token_buf[0] = '\0';
                    has_content = 0;
                }
            } else {
                append_char(&token_buf, &token_len, &token_cap, c);
                has_content = 1;
            }
        }
    }

    if (has_content) {
        if ((size_t)args_len + 1 >= args_cap) {
            args_cap *= 2;
            tokens = realloc(tokens, args_cap * sizeof(char*));
            if (!tokens) { perror("realloc tokens"); exit(EXIT_FAILURE); }
        }
        tokens[args_len++] = strdup(token_buf);
    }

    tokens[args_len] = NULL;
    free(token_buf);
    *count = args_len;
    return tokens;
}

Command* parse_command_line(const char *input) {
    int raw_count = 0;
    char **raw_tokens = tokenize(input, &raw_count);
    if (!raw_tokens || raw_count == 0) {
        if (raw_tokens) free(raw_tokens);
        return NULL;
    }

    Command *cmd = calloc(1, sizeof(Command));
    if (!cmd) {
        perror("calloc Command");
        exit(EXIT_FAILURE);
    }

    char **final_args = malloc((raw_count + 1) * sizeof(char*));
    int final_count = 0;

    for (int i = 0; i < raw_count; ++i) {
        const char *t = raw_tokens[i];

        if (strcmp(t, ">") == 0 || strcmp(t, "1>") == 0) {
            if (i + 1 < raw_count) {
                cmd->stdout_file = strdup(raw_tokens[++i]);
                cmd->stdout_append = 0;
            }
        } else if (strcmp(t, ">>") == 0 || strcmp(t, "1>>") == 0) {
            if (i + 1 < raw_count) {
                cmd->stdout_file = strdup(raw_tokens[++i]);
                cmd->stdout_append = 1;
            }
        } else if (strcmp(t, "2>") == 0) {
            if (i + 1 < raw_count) {
                cmd->stderr_file = strdup(raw_tokens[++i]);
                cmd->stderr_append = 0;
            }
        } else if (strcmp(t, "2>>") == 0) {
            if (i + 1 < raw_count) {
                cmd->stderr_file = strdup(raw_tokens[++i]);
                cmd->stderr_append = 1;
            }
        } else {
            final_args[final_count++] = strdup(t);
        }
    }

    final_args[final_count] = NULL;
    cmd->args = final_args;
    cmd->arg_count = final_count;

    for (int i = 0; i < raw_count; ++i) {
        free(raw_tokens[i]);
    }
    free(raw_tokens);

    if (final_count == 0 && !cmd->stdout_file && !cmd->stderr_file) {
        free_command(cmd);
        return NULL;
    }

    return cmd;
}

void free_command(Command *cmd) {
    if (!cmd) return;
    if (cmd->args) {
        for (int i = 0; i < cmd->arg_count; ++i) {
            if (cmd->args[i]) free(cmd->args[i]);
        }
        free(cmd->args);
    }
    if (cmd->stdout_file) free(cmd->stdout_file);
    if (cmd->stderr_file) free(cmd->stderr_file);
    free(cmd);
}
