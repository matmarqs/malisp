#include <stdio.h>
#include <stdlib.h>

#include <readline/history.h>
#include <readline/readline.h>

int main(int argc, char **argv) {
    puts("malisp version 0.0.0.1");
    puts("Press Ctrl+c to exit");

    while (1) {    
        char *input = readline("lisp> ");
        add_history(input);
        printf("Echo: %s\n", input);
        free(input);
    }

    return 0;
}
