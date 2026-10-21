// unshr.c -- list or unpack an Apogee .SHR installer archive on a host,
// as wolfinst.c does inside C4IX (make test-wolf3d uses it).
//
//   unshr W3DSW14.SHR          list the files
//   unshr W3DSW14.SHR DIR      write them into DIR, names in lower case
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../explode.c"

int main(int argc, char **argv) {
    FILE *f;
    long n;
    unsigned char *d, *out;
    int pos, csize, len, i, bad = 0;
    if (argc < 2 || !(f = fopen(argv[1], "rb"))) { fprintf(stderr, "usage: unshr FILE.SHR [DIR]\n"); return 2; }
    fseek(f, 0, SEEK_END); n = ftell(f); fseek(f, 0, SEEK_SET);
    d = malloc(n);
    if (fread(d, 1, n, f) != (size_t)n) { fprintf(stderr, "unshr: cannot read %s\n", argv[1]); return 1; }
    fclose(f);
    out = malloc(4 << 20);
    pos = 0x3A;
    while (pos + 0x88 + 4 + 0x1C <= n && d[pos]) {
        char name[17], path[512];
        for (i = 0; i < 16 && d[pos + i]; i++) name[i] = tolower(d[pos + i]);
        name[i] = 0;
        csize = d[pos + 0x88] | d[pos + 0x89] << 8 | d[pos + 0x8A] << 16 | d[pos + 0x8B] << 24;
        pos += 0x88 + 4 + 0x1C;
        len = explode(d + pos, csize, out, 4 << 20);
        printf("%-14s %7d -> %7d\n", name, csize, len);
        if (len < 0) bad = 1;
        else if (argc > 2) {
            snprintf(path, sizeof path, "%s/%s", argv[2], name);
            if (!(f = fopen(path, "wb")) || fwrite(out, 1, len, f) != (size_t)len) { fprintf(stderr, "unshr: cannot write %s\n", path); return 1; }
            fclose(f);
        }
        pos += csize;
    }
    if (pos != n) { fprintf(stderr, "unshr: stopped at %d of %ld bytes\n", pos, n); bad = 1; }
    return bad;
}
