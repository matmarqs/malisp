#include <stdio.h>
#include <editline.h>
#include <stdlib.h>
#include <stdbool.h>
#include "reader.h"
#include "printer.h"

char *mal_read() {
    char *input = readline("user> ");
    if (input) {
        add_history(input);
    }
    return input;
}

char *mal_eval(char *input) {
    return input;
}

char *mal_print(char *input) {
    puts(input);
    return input;
}

bool mal_rep() {
    char *input = mal_read();
    if (!input) {
        puts("");
        return false;
    }
    mal_t root = read_str(input);
    pr_str(root);
    //input = mal_print(input);
    free(input);
    return true;
}

int main() {
    bool running = true;
    while (running) {
        running = mal_rep();
    }
    return 0;
}
