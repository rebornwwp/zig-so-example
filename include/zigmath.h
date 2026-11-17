#ifndef ZIGMATH_H
#define ZIGMATH_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int32_t  zm_add(int32_t a, int32_t b);
int32_t  zm_sum(const int32_t *items, size_t len);
char    *zm_greet(const char *name);  /* 调用方必须用 zm_string_free 释放 */
void     zm_string_free(char *ptr);
uint32_t zm_version(void);

#ifdef __cplusplus
}
#endif

#endif /* ZIGMATH_H */
