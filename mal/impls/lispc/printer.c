#include <stdio.h>
#include "printer.h"

void pr_str(mal_t *mal_object) {
    switch (mal_object->type) {
    case MAL_SYMBOL:
        printf("%.*s", mal_object->data.symbol.str_len, mal_object->data.symbol.str);
        break;
    case MAL_NUMBER:
        printf("%lld", (long long)mal_object->data.number); break;
    case MAL_LIST:
        putchar('(');
        for (int i = 0; i < mal_object->data.list->top + 1; i++) {
            pr_str(&mal_object->data.list->items[i]);
            if (i != mal_object->data.list->top) {
                putchar(' ');
            }
        }
        putchar(')');
        break;
    }
}
