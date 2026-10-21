// c4file.c -- the game's files, whole, in memory: C4IX has no seek.
//
// Wolf4SDL reads its data with fopen, fseek, ftell and fread, and writes
// its config and saved games with fwrite. Here "rb" reads the whole file
// into memory at fopen, so fseek and ftell are arithmetic; "wb" gathers
// the bytes and writes them out at fclose. c4file.h points the game's
// calls here, in the gcc build as well, so the host tests this too.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c4file.h"

static int grow(WFILE *f, int need) {
    char *b;
    int cap;
    if (need <= f->cap) return 1;
    cap = f->cap ? f->cap : 4096;
    while (cap < need) cap = cap * 2;
    b = (char *)realloc(f->buf, cap);
    if (!b) return 0;
    f->buf = b;
    f->cap = cap;
    return 1;
}

WFILE *w_fopen(const char *path, const char *mode) {
    WFILE *f;
    FILE *h;
    int n;
    f = (WFILE *)calloc(1, sizeof(WFILE));
    if (!f) return NULL;
    f->writing = mode[0] == 'w';
    strncpy(f->path, path, sizeof(f->path) - 1);
    if (f->writing) return f;
    h = fopen(path, "rb");
    if (!h) { free(f); return NULL; }
    while (1) {
        if (!grow(f, f->len + 65536)) { fclose(h); free(f->buf); free(f); return NULL; }
        n = fread(f->buf + f->len, 1, 65536, h);
        if (n <= 0) break;
        f->len += n;
    }
    fclose(h);
    return f;
}

size_t w_fread(void *p, size_t size, size_t n, WFILE *f) {
    int want, have;
    if (size == 0) return 0;
    want = size * n;
    have = f->len - f->pos;
    if (have < want) { n = have / size; want = n * size; }
    memcpy(p, f->buf + f->pos, want);
    f->pos += want;
    return n;
}

size_t w_fwrite(const void *p, size_t size, size_t n, WFILE *f) {
    int bytes;
    bytes = size * n;
    if (!f->writing || !grow(f, f->pos + bytes)) return 0;
    memcpy(f->buf + f->pos, p, bytes);
    f->pos += bytes;
    if (f->pos > f->len) f->len = f->pos;
    return n;
}

int w_fseek(WFILE *f, long off, int whence) {
    long to;
    to = whence == SEEK_SET ? off : whence == SEEK_CUR ? f->pos + off : f->len + off;
    if (to < 0) return -1;
    if (to > f->len && !f->writing) to = f->len;
    if (to > f->len && !grow(f, to)) return -1;
    while (f->len < to) { f->buf[f->len] = 0; ++f->len; }
    f->pos = to;
    return 0;
}

long w_ftell(WFILE *f) { return f->pos; }

int w_fclose(WFILE *f) {
    FILE *h;
    int ok;
    ok = 0;
    if (f->writing) {
        h = fopen(f->path, "wb");
        if (!h || (f->len && fwrite(f->buf, 1, f->len, h) != f->len)) ok = -1;
        if (h) fclose(h);
    }
    free(f->buf);
    free(f);
    return ok;
}
