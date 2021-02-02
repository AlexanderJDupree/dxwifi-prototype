/**
 * DxWifi collection of utility macros/functions
 */

#ifndef DXWIFI_UTIIITY_H
#define DXWIFI_UTIIITY_H

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

/* The first occurrence of EXPR is not evaluated due to the sizeof,
   but will trigger any pedantic warnings masked by the __extension__
   for the second occurrence. */
#define assert_M(expr, msg, ...)                                                \
    ((void) sizeof ((expr) ? 1 : 0), __extension__ ({                           \
        if (expr)								                                \
            ; /* empty */							                            \
        else								                                    \
            __assert_M (true, #expr, __FILE__, __LINE__, msg, ##__VA_ARGS__);   \
    }))


#define assert_continue(expr, msg, ...)                                         \
    ((void) sizeof ((expr) ? 1 : 0), __extension__ ({                           \
        if (expr)								                                \
            ; /* empty */							                            \
        else								                                    \
            __assert_M (false, #expr, __FILE__, __LINE__, msg, ##__VA_ARGS__);  \
    }))

#define assert_always(msg, ...) assert_M(true, msg, ...)
#define assert_not_null(ptr) assert_M((ptr != NULL), "%s is NULL", #ptr)


static void __assert_M(bool exit, const char* expr, const char* file, int line, const char* msg, ...) {
    va_list args;
    fprintf(stderr, "%s:%d Assertion `%s` failed : ", file, line, expr);
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    fprintf(stderr, "\n");
    if( exit ) {
        abort();
    }
}

#ifdef NDEBUG
#define debug_assert(expr, msg, ...) (void)0
#define debug_assert_always(msg, ...) (void)0
#define debug_assert_not_null(ptr) (void)0
#else
#define assert_debug(expr, msg, ...) assert_M(expr, msg, ##__VA_ARGS__)
#define debug_assert_always(msg, ...) assert_always(msg, ##__VA_ARGS__)
#define debug_assert_not_null(ptr) assert_not_null(ptr)
#endif

#endif // DXWIFI_UTIIITY_H