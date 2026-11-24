
# minishell

A UNIX command-line interpreter (shell) written in **C** from scratch, developed as part of an **Operating Systems** university course.

## Features

- [x] **Command Execution & Pipelines:** Supports single commands and multi-command pipelines (`cmd1 | cmd2 | ... | cmdN`) using dynamic pipe allocation (`fork`, `execv`, `pipe`, `dup2`, `waitpid`).
- [x] **I/O Redirection:**
  - Standard input redirection (`< file`) on the first command.
  - Standard output (`> file`) redirection on the last command.
- [ ] **Error Redirection:** Standard error redirection (`>& file`) on the last command.
- [ ] **Built-in Commands:** Internal execution of shell built-ins (`cd`, `exit`, `umask`, `jobs`, `bg`) directly in the parent process.
- [ ] **Process & Signal Control:** Background job execution (`&`) and signal handling (`SIGINT`, `SIGQUIT`).

## Build & Run

The project relies on the provided lexical/syntactic parser library (`parser.h` and `libparser.a`). Compile with strict warning flags:

```bash
gcc -Wall -Wextra minishell.c libparser.a -o minishell -static
```
Start the shell:

Bash
```
./minishell
```
Usage Examples

Bash
```
msh> ls -l
msh> ls -l | grep minishell | wc -l
msh> cat < input.txt | sort > output.txt
```

## Technologies & System Calls
Language: C (ANSI/POSIX)

Core Syscalls: fork, execv, waitpid, pipe, dup2, open, close, chdir, umask, signal
