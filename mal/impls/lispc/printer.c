#include <stdio.h>
#include "printer.h"

void pr_str(mal_t mal_object) {
    switch (mal_object.type) {
    case MAL_OP:
        putchar(mal_object.data.op); putchar(' '); break;
    case MAL_NUMBER:
        printf("%lld ", (long long)mal_object.data.number); break;
    case MAL_LIST:
        putchar('('); putchar(' ');
        for (int i = 0; i < mal_object.data.list->top + 1; i++) {
            pr_str(mal_object.data.list->items[i]);
            if (i != mal_object.data.list->top) {
                putchar(' ');
            }
        }
        putchar(')');
        break;
    }

}
