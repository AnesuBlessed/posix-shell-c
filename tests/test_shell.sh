#!/usr/bin/env bash
# ==============================================================================
# posix-shell automated test suite
# Validates builtins, path resolution, quoting, and file descriptor redirection
# ==============================================================================

set -euo pipefail

SHELL_BIN="./posix-shell"
TMP_DIR=$(mktemp -d)
trap 'rm -rf "$TMP_DIR"' EXIT

echo "=== Running posix-shell Test Suite ==="

# Helper function
run_test() {
    local name="$1"
    local input="$2"
    local expected="$3"

    local actual
    actual=$(echo -e "$input" | "$SHELL_BIN" 2>&1 || true)
    # Strip any trailing whitespace
    actual=$(echo "$actual" | sed -e 's/[[:space:]]*$//')
    expected=$(echo "$expected" | sed -e 's/[[:space:]]*$//')

    if [ "$actual" = "$expected" ]; then
        echo -e "  \033[32m✓\033[0m $name"
    else
        echo -e "  \033[31m✗\033[0m $name"
        echo "    Expected: '$expected'"
        echo "    Got:      '$actual'"
        exit 1
    fi
}

# 1. Built-in echo
run_test "builtin echo simple" "echo hello world" "hello world"
run_test "builtin echo multiple spaces" "echo   hello    world  " "hello world"

# 2. Built-in type
run_test "builtin type builtins" "type echo exit type pwd cd help" \
"echo is a shell builtin
exit is a shell builtin
type is a shell builtin
pwd is a shell builtin
cd is a shell builtin
help is a shell builtin"

run_test "builtin type external" "type ls" "ls is $(which ls)"
run_test "builtin type not found" "type nonexistentcmd12345" "nonexistentcmd12345: not found"

# 3. Built-in pwd & cd
CURRENT_DIR=$(pwd)
run_test "builtin pwd" "pwd" "$CURRENT_DIR"
run_test "builtin cd and pwd" "cd /tmp\npwd" "/tmp"

# 4. Quoting preservation
run_test "single quotes preserve spaces" "echo 'hello    world'" "hello    world"
run_test "double quotes simple" 'echo "hello world"' "hello world"
run_test "double quotes escape quote" 'echo "hello \"world\""' 'hello "world"'
run_test "double quotes escape backslash" 'echo "hello \\ world"' 'hello \ world'

# 5. Redirection tests
echo "Testing I/O Redirection..."
echo "echo Redirection Test > $TMP_DIR/out.txt" | "$SHELL_BIN"
if [ "$(cat "$TMP_DIR/out.txt")" = "Redirection Test" ]; then
    echo -e "  \033[32m✓\033[0m stdout redirection (>)"
else
    echo -e "  \033[31m✗\033[0m stdout redirection (>)"
    exit 1
fi

echo "echo Line 2 >> $TMP_DIR/out.txt" | "$SHELL_BIN"
EXPECTED_APPEND="Redirection Test
Line 2"
if [ "$(cat "$TMP_DIR/out.txt")" = "$EXPECTED_APPEND" ]; then
    echo -e "  \033[32m✓\033[0m stdout append redirection (>>)"
else
    echo -e "  \033[31m✗\033[0m stdout append redirection (>>)"
    exit 1
fi

echo "ls /nonexistent_dir_xyz_123 2> $TMP_DIR/err.txt" | "$SHELL_BIN" || true
if [ -s "$TMP_DIR/err.txt" ]; then
    echo -e "  \033[32m✓\033[0m stderr redirection (2>)"
else
    echo -e "  \033[31m✗\033[0m stderr redirection (2>)"
    exit 1
fi

echo "=== All 10/10 tests passed successfully! ==="
