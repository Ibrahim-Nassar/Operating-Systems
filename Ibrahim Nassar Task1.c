#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>

#define MAX_LINE 1024
#define MAX_ARGS 100

int shell_cd(char **args);
int shell_dir(char **args);
int shell_environ(char **args);
int shell_set(char **args);
int shell_echo(char **args);
int shell_help(char **args);
int shell_pause_shell(char **args);
int shell_quit(char **args);

char *builtin_str[] = {
    "cd",
    "dir",
    "environ",
    "set",
    "echo",
    "help",
    "pause",
    "quit"
};

int (*builtin_func[]) (char **) = {
    &shell_cd,
    &shell_dir,
    &shell_environ,
    &shell_set,
    &shell_echo,
    &shell_help,
    &shell_pause_shell,
    &shell_quit
};

int num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

int shell_cd(char **args) {
    if (args[1] == NULL) {
        char *cwd = getenv("PWD");
        if (cwd)
            printf("%s\n", cwd);
        else
            fprintf(stderr, "PWD not set.\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("cd");
        } else {
            char cwd[1024];
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                setenv("PWD", cwd, 1);
            }
        }
    }
    return 1;
}

int shell_dir(char **args) {
    char *dir = args[1];
    if (dir == NULL) {
        dir = ".";
    }
    DIR *d = opendir(dir);
    if (d == NULL) {
        perror("dir");
        return 1;
    }
    struct dirent *entry;
    while ((entry = readdir(d)) != NULL) {
        printf("%s ", entry->d_name);
    }
    printf("\n");
    closedir(d);
    return 1;
}

int shell_environ(char **args) {
    extern char **environ;
    for (char **env = environ; *env != 0; env++) {
        printf("%s\n", *env);
    }
    return 1;
}

int shell_set(char **args) {
    if (args[1] == NULL || args[2] == NULL) {
        fprintf(stderr, "Usage: set VARIABLE VALUE\n");
        return 1;
    }
    if (setenv(args[1], args[2], 1) != 0) {
        perror("set");
    }
    return 1;
}

int shell_echo(char **args) {
    if (args[1] == NULL) {
        printf("\n");
        return 1;
    }
    char *output = strdup(args[1]);
    if (output == NULL) {
        perror("echo");
        return 1;
    }
    int index = 0;
    int len = strlen(output);
    for (int i = 1; i < MAX_ARGS && args[i] != NULL; i++) {
        printf("%s ", args[i]);
    }
    printf("\n");
    free(output);
    return 1;
}

int shell_help(char **args) {
    printf("Simple Shell Implementation\n");
    printf("Built-in commands:\n");
    printf("  cd [DIRECTORY]      Change directory or display current directory\n");
    printf("  dir DIRECTORY       List contents of DIRECTORY\n");
    printf("  environ             List all environment variables\n");
    printf("  set VARIABLE VALUE  Set or create an environment variable\n");
    printf("  echo [COMMENT]      Display COMMENT\n");
    printf("  help                Display this help message\n");
    printf("  pause               Pause the shell until 'Enter' is pressed\n");
    printf("  quit                Exit the shell\n");
    printf("External commands are also supported.\n");
    return 1;
}

int shell_pause_shell(char **args) {
    printf("Press Enter to continue...");
    while (getchar() != '\n');
    return 1;
}

int shell_quit(char **args) {
    exit(0);
}

int launch(char **args, int background, char *input_file, char *output_file, int append) {
    pid_t pid, wpid;
    int status;
    pid = fork();
    if (pid == 0) {
        if (input_file != NULL) {
            int fd0 = open(input_file, O_RDONLY);
            if (fd0 < 0) {
                perror("Input redirection");
                exit(EXIT_FAILURE);
            }
            dup2(fd0, STDIN_FILENO);
            close(fd0);
        }
        if (output_file != NULL) {
            int fd1;
            if (append)
                fd1 = open(output_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
            else
                fd1 = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd1 < 0) {
                perror("Output redirection");
                exit(EXIT_FAILURE);
            }
            dup2(fd1, STDOUT_FILENO);
            close(fd1);
        }
        if (execvp(args[0], args) == -1) {
            perror("exec");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("fork");
    } else {
        if (!background) {
            do {
                wpid = waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        } else {
            printf("Process running in background with PID %d\n", pid);
        }
    }
    return 1;
}

int execute(char **args) {
    if (args[0] == NULL) {
        return 1;
    }

    for (int i = 0; i < num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    int background = 0;
    char *input_file = NULL;
    char *output_file = NULL;
    int append = 0;

    int i = 0;
    while (args[i] != NULL) {
        if (strcmp(args[i], "<") == 0) {
            if (args[i + 1] != NULL) {
                input_file = args[i + 1];
                for (int j = i; args[j + 2] != NULL; j++) {
                    args[j] = args[j + 2];
                }
                args[i + 2] = NULL;
            } else {
                fprintf(stderr, "Expected input file after '<'\n");
                return 1;
            }
        } else if (strcmp(args[i], ">") == 0) {
            if (args[i + 1] != NULL) {
                output_file = args[i + 1];
                append = 0;
                for (int j = i; args[j + 2] != NULL; j++) {
                    args[j] = args[j + 2];
                }
                args[i + 2] = NULL;
            } else {
                fprintf(stderr, "Expected output file after '>'\n");
                return 1;
            }
        } else if (strcmp(args[i], ">>") == 0) {
            if (args[i + 1] != NULL) {
                output_file = args[i + 1];
                append = 1;
                for (int j = i; args[j + 2] != NULL; j++) {
                    args[j] = args[j + 2];
                }
                args[i + 2] = NULL;
            } else {
                fprintf(stderr, "Expected output file after '>>'\n");
                return 1;
            }
        } else if (strcmp(args[i], "&") == 0) {
            background = 1;
            args[i] = NULL;
        } else {
            i++;
        }
    }

    return launch(args, background, input_file, output_file, append);
}

char *read_line(FILE *input_stream) {
    char *line = NULL;
    size_t bufsize = 0;
    if (getline(&line, &bufsize, input_stream) == -1) {
        if (feof(input_stream)) {
            exit(0);
        } else {
            perror("readline");
            exit(EXIT_FAILURE);
        }
    }
    size_t len = strlen(line);
    if (len > 0 && line[len -1] == '\n') {
        line[len -1] = '\0';
    }
    return line;
}

char **split_line(char *line) {
    int bufsize = MAX_ARGS, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if (!tokens) {
        fprintf(stderr, "Allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, " \t");
    while (token != NULL) {
        tokens[position++] = token;

        if (position >= bufsize) {
            bufsize += MAX_ARGS;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "Allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, " \t");
    }
    tokens[position] = NULL;
    return tokens;
}

void shell_loop(FILE *input_stream) {
    char cwd[1024];
    char *line;
    char **args;
    int status = 1;

    while (1) {
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s$ ", cwd);
        } else {
            perror("getcwd");
            printf("$ ");
        }

        line = read_line(input_stream);
        args = split_line(line);
        status = execute(args);
        free(line);
        free(args);
    }
}

int main(int argc, char **argv) {
    FILE *input_stream = stdin;

    if (argc == 2) {
        input_stream = fopen(argv[1], "r");
        if (input_stream == NULL) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }
    } else if (argc > 2) {
        fprintf(stderr, "Usage: %s [batchfile]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (getenv("PWD") == NULL) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            setenv("PWD", cwd, 1);
        }
    }

    shell_loop(input_stream);

    if (input_stream != stdin) {
        fclose(input_stream);
    }

    return EXIT_SUCCESS;
}
