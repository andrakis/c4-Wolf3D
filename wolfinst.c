// wolfinst.c -- the shareware episode, unpacked at first run.
//
// C4IX ships Apogee's shareware package as it was distributed,
// 1wolf14.zip, unmodified (its VENDOR.DOC asks exactly that). In it
// W3DSW14.SHR, the installer's archive: a 0x3A-byte header, then for each
// file a 16-byte name, 0x88 bytes in all to its compressed size, 0x1C
// more, and the file compressed with PKWARE's DCL "implode" (explode.c).
// The first time the game runs where there is no vswap.wl1, it reads the
// zip (deflate, libarc's arc_inflate), walks the archive, and writes each
// file there under its name in lower case, which is how Wolf4SDL looks.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "archive.h"
// explode.c: PKWARE DCL "implode" data, decompressed
int explode(unsigned char *in, int inlen, unsigned char *out, int outcap);

enum { ZIP_LOCAL = 0x04034b50, ZIP_CENTRAL = 0x02014b50, ZIP_END = 0x06054b50 };

static int le16(unsigned char *p) { return p[0] | (p[1] << 8); }
static int le32(unsigned char *p) { return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24); }

static unsigned char *whole(char *path, int *len) {
    FILE *f;
    unsigned char *buf;
    int cap, n;
    f = fopen(path, "rb");
    if (!f) return NULL;
    cap = 1 << 20; *len = 0;
    buf = (unsigned char *)malloc(cap);
    while (buf) {
        n = fread(buf + *len, 1, cap - *len, f);
        if (n <= 0) break;
        *len += n;
        if (*len == cap) { cap = cap * 2; buf = (unsigned char *)realloc(buf, cap); }
    }
    fclose(f);
    return buf;
}

// The member NAME of the zip in Z (ZLEN bytes), inflated: *LEN bytes, or NULL
static unsigned char *zip_member(unsigned char *z, int zlen, char *name, int *len) {
    int e, n, at, i, method, csize, usize, crc, nlen, xlen, clen, local, used;
    unsigned char *out;
    // the end record: the last 22 bytes, unless there is a comment
    e = zlen - 22;
    while (e >= 0 && le32(z + e) != ZIP_END) --e;
    if (e < 0) return NULL;
    n = le16(z + e + 10);
    at = le32(z + e + 16);
    for (i = 0; i < n; i++) {
        if (at + 46 > zlen || le32(z + at) != ZIP_CENTRAL) return NULL;
        method = le16(z + at + 10);
        crc = le32(z + at + 16);
        csize = le32(z + at + 20);
        usize = le32(z + at + 24);
        nlen = le16(z + at + 28);
        xlen = le16(z + at + 30);
        clen = le16(z + at + 32);
        local = le32(z + at + 42);
        if (nlen == (int)strlen(name) && !memcmp(z + at + 46, name, nlen)) {
            if (local + 30 > zlen || le32(z + local) != ZIP_LOCAL) return NULL;
            local = local + 30 + le16(z + local + 26) + le16(z + local + 28);
            if (local + csize > zlen) return NULL;
            out = (unsigned char *)malloc(usize + 1);
            if (!out) return NULL;
            if (method == 0) memcpy(out, z + local, usize);
            else if (method != 8 || arc_inflate((char *)z + local, csize, (char *)out, usize, &used) != usize) {
                free(out); return NULL;
            }
            if ((arc_crc32(0, (char *)out, usize) & 0xFFFFFFFF) != (crc & 0xFFFFFFFF)) { free(out); return NULL; }
            *len = usize;
            return out;
        }
        at = at + 46 + nlen + xlen + clen;
    }
    return NULL;
}

// Unpack ZIP's episode here. The files written, or -1.
int WolfInstall(char *zip) {
    unsigned char *z, *shr, *out;
    int zlen, slen, pos, csize, len, files, i;
    char name[20];
    FILE *f;
    z = whole(zip, &zlen);
    if (!z) return -1;
    printf("Unpacking the shareware episode from %s...\n", zip);
    shr = zip_member(z, zlen, "W3DSW14.SHR", &slen);
    free(z);
    if (!shr) { printf("wolf3d: %s has no W3DSW14.SHR that inflates\n", zip); return -1; }
    out = (unsigned char *)malloc(1 << 20);
    files = 0;
    pos = 0x3A;
    while (out && pos + 0x88 + 4 + 0x1C <= slen && shr[pos]) {
        for (i = 0; i < 16 && shr[pos + i]; i++) {
            name[i] = shr[pos + i];
            if (name[i] >= 'A' && name[i] <= 'Z') name[i] = name[i] + 32;
        }
        name[i] = 0;
        csize = le32(shr + pos + 0x88);
        pos = pos + 0x88 + 4 + 0x1C;
        if (csize < 0 || pos + csize > slen) break;
        len = explode(shr + pos, csize, out, 1 << 20);
        pos = pos + csize;
        if (len < 0) { printf("wolf3d: %s does not explode\n", name); files = -1; break; }
        // the DOS programs are of no use here; the rest is the game
        if (strstr(name, ".exe")) continue;
        f = fopen(name, "wb");
        if (!f || fwrite(out, 1, len, f) != len) { printf("wolf3d: cannot write %s\n", name); files = -1; if (f) fclose(f); break; }
        fclose(f);
        printf("  %-12s %7d bytes\n", name, len);
        ++files;
    }
    free(out);
    free(shr);
    return files;
}

// Where the game's files are: here, or WOLF_HOME, where C4IX ships them
// (the game changes there); unpacked from 1wolf14.zip the first time.
#define WOLF_HOME "/usr/games/wolf3d"
#ifdef WOLF_FULL
// The registered game (wolf3d6): its files are the person's own, so none
// ship and none are unpacked. They are where it is run, or on the host's
// drive 0, where the page's Uploads put them -- which C4IX opens when its
// own tree has no such file (sys_open), though stat() does not see them.
void WolfData(void) { }
#else
void WolfData(void) {
    struct stat st;
    if (stat("vswap.wl1", &st) == 0) return;
    if (stat("1wolf14.zip", &st) != 0 && chdir(WOLF_HOME) != 0) return;
    if (stat("vswap.wl1", &st) == 0) return;
    if (stat("1wolf14.zip", &st) == 0) WolfInstall("1wolf14.zip");
}
#endif
