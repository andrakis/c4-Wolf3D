// plat-host.c -- c4sdl's platform on a host, built with gcc: no window.
//
// WOLF_DUMP=N[,N...]  writes frame N to wolf-N.ppm, to look at.
// WOLF_KEYS=F:SC[+-],...  presses (+) or releases (-) scancode SC when
//                       frame F has been shown, to drive the menus.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "SDL.h"
#include "plat.h"

extern unsigned int c4sdl_frames;
static unsigned int frames;
static char *keys;

int plat_init(void) { keys = getenv("WOLF_KEYS"); return 0; }
void plat_quit(void) { }
void plat_open(const char *title, int w, int h) { }

static int wanted(unsigned int n) {
    char *s = getenv("WOLF_DUMP");
    while (s && *s) {
        if ((unsigned int)atoi(s) == n) return 1;
        s = strchr(s, ',');
        if (s) ++s;
    }
    return 0;
}

void plat_present(unsigned char *pixels, int w, int h, int pitch, SDL_Color *pal) {
    char name[64];
    FILE *f;
    int x, y;
    ++frames;
    if (!wanted(frames)) return;
    snprintf(name, sizeof name, "wolf-%u.ppm", frames);
    f = fopen(name, "wb");
    if (!f) return;
    fprintf(f, "P6\n%d %d\n255\n", w, h);
    for (y = 0; y < h; y++)
        for (x = 0; x < w; x++) {
            SDL_Color *c = &pal[pixels[y * pitch + x]];
            fputc(c->r, f); fputc(c->g, f); fputc(c->b, f);
        }
    fclose(f);
}

int plat_event(int *kind, int *sc, int *ch) {
    // the next scripted key whose frame has come
    int f, n, k;
    char *end;
    if (!keys || !*keys) return 0;
    f = strtol(keys, &end, 10);
    if ((unsigned int)f > frames || *end != ':') return 0;
    n = strtol(end + 1, &end, 10);
    k = *end == '-' ? PLAT_UP : PLAT_DOWN;
    if (*end == '+' || *end == '-') ++end;
    keys = *end == ',' ? end + 1 : end;
    *kind = k;
    *sc = n;
    *ch = 0;
    return 1;
}

unsigned int plat_ticks(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1000 + t.tv_nsec / 1000000;
}

// no sound card: the game runs silent, as it did with no audio device
int  plat_snd_open(int rate) { return 0; }
void plat_snd_close(void) { }
void plat_snd_opl(int reg, int val) { }
void plat_snd_wait(int samples) { }
int  plat_snd_ahead(void) { return 0; }
void plat_snd_play(int ch, unsigned char *data, int len, int rate, int vol) { }
void plat_snd_stop(int ch) { }
void plat_snd_volume(int ch, int vol) { }
int  plat_snd_busy(void) { return 0; }

void plat_delay(unsigned int ms) {
    struct timespec t;
    t.tv_sec = ms / 1000;
    t.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&t, NULL);
}
