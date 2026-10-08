#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* here we are going to use a common strategy in C 
    which allows you to reallocate memory based
    on whether or not they exceed the memory block */

int lsh_num_builtins(void);

#define LSH_RL_BUFSIZE 1024
char *lsh_read_line(void) {
    int bufsize = LSH_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if(!buffer) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while(1) {
        c = getchar();

        // if end of file, replace it with null character and return 
        if(c == EOF || c == '\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }
        position++;

        // if buffer is exceeded, reallocate
        if(position >= bufsize) {
            bufsize += LSH_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if(!buffer) {
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
char **lsh_split_line(char *line) {
    int bufsize = LSH_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if(!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, LSH_TOK_DELIM);
    while(token != NULL) {
        tokens[position] = token;
        position++;

        if(position >= bufsize) {
            bufsize += LSH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if(!tokens) {
                fprintf(stderr, "lsh: allocation error");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, LSH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

int lsh_launch(char **args) {
    pid_t pid, wpid;
    int status;

    pid = fork();

    if(pid == 0) {
        if(execvp(args[0], args) == -1) {
            perror("lsh");
        }
        exit(EXIT_FAILURE);
    }
    else if(pid < 0) {
        /* means error forking because a process cannot
        be less than 0 */ 
        perror("lsh");
    } else {
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while(!WIFEXITED(status) && !WIFSIGNALED(status));
    } 
    return 1;
}

int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);
int lsh_app(char **args);

char *builtin_str[] = {
    "cd",
    "help",
    "exit",
    "app"
};

int(*builtin_func[]) (char**) = {
    &lsh_cd,
    &lsh_help,
    &lsh_exit,
    &lsh_app
};

int lsh_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

int lsh_cd(char **args) {
    if(args[1] == NULL) {
        fprintf(stderr, "lsh: expected argument to \"cd\"\n");
    } else {
        if(chdir(args[1]) != 0) {
            perror("lsh");
        }
    }
    return 1;
}

int lsh_app(char **args) {
    char **newArgs = malloc(sizeof(char *) * 4);

    if(args[1] == NULL) {
        fprintf(stderr, "app: expected argument");
    }
    
    newArgs[0] = "open";
    newArgs[1] = "-a";
    newArgs[2] = args[1];
    newArgs[3] = NULL;

    lsh_launch(newArgs);

    free(newArgs);

    return 1;
}

int lsh_help(char **args) {
    printf("Collin Le's LSH\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in commands:\n");

    for(int i = 0; i < lsh_num_builtins(); i++) {
        printf(" %s\n", builtin_str[i]);
    }

    printf("Use the man command for information on ther programs.\n");
    return 1;
}

int lsh_exit(char **args) {
    printf("Exiting...");
    return 0;
}

int lsh_execute(char **args) {
    if(args[0] == NULL) {
        return 1;
    }

    for(int i = 0; i < lsh_num_builtins(); i++) {
        if(strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }
    return lsh_launch(args);
}

void lsh_loop(void) {
    char *line;
    char **args;
    int status;

    do {
        printf("> ");
        line = lsh_read_line();
        args = lsh_split_line(line);
        status = lsh_execute(args);

        free(line);
        free(args);
    } while(status);
}

int main(int argc, char **argv) {
    lsh_loop();

    return EXIT_SUCCESS;
}