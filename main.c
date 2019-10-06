#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define SHELL_RL_BUFSIZE 2048
#define SHELL_TOK_BUFSIZE 64
#define  SHELL_TOK_DELIM "\t\r\n\a"

//shell division
//read      -read input
//parse     -separate program and args
//execute   -run parsed command

//function declaration
int shell_cd(char **args);

int shell_help(char **args);

int shell_exit(char **args);

//list of commands
char *builtin_str[] = {
        "cd",
        "help",
        "exit"
};

int (*builtin_func[])(char **) ={
        &shell_cd,
        &shell_help,
        &shell_exit
};

int shell_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

int shell_launch(char **args) {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0) {
        //child process
        if (execvp(args[0], args) == -1) {
            perror("shell");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("shell");
    } else {
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}

int shell_execute(char **args) {
    int i;

    if (args[0] == NULL) {
        return 1;
    }
    for (i = 0; i < shell_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }
    return shell_launch(args);
}

int shell_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "shell: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("shell");
        }
    }
    return 1;
}

int shell_help(char **args) {
    int i;
    printf("First Shell Test\n");
    printf("type commands and press enter\n");

    for (i = 0; i < shell_num_builtins(); i++) {
        printf("%s\n", builtin_str[i]);
    }

    printf("ok --------------");
    return 1;
}

int shell_exit(char **args) {
    return 0;
}



char **shell_split_line(char *line) {
    int bufsize = SHELL_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char *));
    char *token;

    if (!tokens) {
        fprintf(stderr, "shell: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, SHELL_TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += SHELL_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char *));
            if (!tokens) {
                fprintf(stderr, "shell: allocation error");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, SHELL_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}


char *shell_read_line(void) {
    int bufsize = SHELL_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if (!buffer) {
        fprintf(stderr, "shell: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        //read
        c = getchar();

        //end of file equals null and return
        if (c == EOF || c == '\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }
        position++;
    }

    if (position >= bufsize) {
        bufsize += SHELL_RL_BUFSIZE;
        buffer = realloc(buffer, bufsize);
        if (!buffer) {
            fprintf(stderr, "shell: allocation error\n");
            exit(EXIT_FAILURE);
        }
    }
}

/*
char *shell_read_line(void) {
    char *line = NULL;
    ssize_t bufsize = 0; //allocate buffer
    getline(&line, &bufsize, stdin);
    return line;
}
 */

void shell_loop(void) {
    char *line;
    char **args;
    int status;

    do {
        printf("--> ");
        line = shell_read_line();
        args = shell_split_line(line);
        status = shell_execute(args);

        free(line);
        free(args);
    } while (status);
}


int main(int argc, char **argv) {
    shell_loop();
    return EXIT_SUCCESS;
}