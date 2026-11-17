#include <stdio.h>
#include <stdlib.h>

#include "zigmath.h"

int main(void) {
    printf("zigmath version: %u\n", zm_version());

    printf("10 + 20 = %d\n", zm_add(10, 20));

    int32_t data[] = {1, 2, 3, 4, 5};
    printf("sum([1..5]) = %d\n", zm_sum(data, sizeof(data) / sizeof(data[0])));

    char *msg = zm_greet("C caller");
    if (msg == NULL) {
        fprintf(stderr, "zm_greet failed\n");
        return EXIT_FAILURE;
    }
    printf("%s\n", msg);
    zm_string_free(msg);

    return EXIT_SUCCESS;
}
