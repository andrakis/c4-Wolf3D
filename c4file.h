// c4file.h -- files read and written whole (c4file.c), for a machine
// without seek. With C4FILE_MACROS defined first, the game's fopen and
// the rest are these.

#ifndef C4FILE_H
#define C4FILE_H 1

typedef struct WFILE {
    char *buf;
    int len, cap, pos;
    int writing;
    char path[256];
} WFILE;

WFILE *w_fopen(const char *path, const char *mode);
size_t w_fread(void *p, size_t size, size_t n, WFILE *f);
size_t w_fwrite(const void *p, size_t size, size_t n, WFILE *f);
int    w_fseek(WFILE *f, long off, int whence);
long   w_ftell(WFILE *f);
int    w_fclose(WFILE *f);

#ifdef C4FILE_MACROS
#define FILE   WFILE
#define fopen  w_fopen
#define fread  w_fread
#define fwrite w_fwrite
#define fseek  w_fseek
#define ftell  w_ftell
#define fclose w_fclose
#endif

#endif
