#ifndef UTILS_H
#define UTILS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

// Сборщик мусора (простой)
#define GC_MAX 10000
static void* gc_objects[GC_MAX];
static int gc_count = 0;

static inline void* safe_alloc(size_t size) {
    void* p = calloc(1, size);
    if (!p) { fprintf(stderr, "💀 Фатальная ошибка: память кончилась\n"); exit(1); }
    if (gc_count < GC_MAX) gc_objects[gc_count++] = p;
    return p;
}

static inline void gc_free_all() {
    for (int i = 0; i < gc_count; i++) free(gc_objects[i]);
    gc_count = 0;
}

static inline char* str_dup(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s) + 1;
    char* c = safe_alloc(len);
    memcpy(c, s, len);
    return c;
}

static inline char* str_fmt(const char* fmt, ...) {
    char buf[4096];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    return str_dup(buf);
}

// Проверка на float
static inline int is_float_str(const char* s) {
    int dots = 0;
    for (int i = 0; s[i]; i++) {
        if (s[i] == '.') dots++;
        if (dots > 1) return 0;
    }
    return dots == 1;
}
#endif