// plat-bare.c -- c4sdl's platform with no operating system under it, or
// with C4DOS: the program is the machine's (libc's bare.c), and draws on
// libjs's display itself (plat-gui.c). A --timedemo runs without the
// display, as everywhere: its frames are checksummed, not shown.

#include "SDL.h"
#include "plat.h"

extern int c4sdl_timedemo;
extern int c4sdl_show;

static int shown;

int plat_init(void) { return 0; }
void plat_quit(void) { pg_close(); }

void plat_open(const char *title, int w, int h) {
    if (!c4sdl_timedemo || c4sdl_show) shown = pg_open(title, w, h);
}

void plat_present(unsigned char *pixels, int w, int h, int pitch, SDL_Color *pal) {
    if (shown) pg_present(pixels, w, h, pitch, pal);
}

int plat_event(int *kind, int *sc, int *ch) { return shown ? pg_event(kind, sc, ch) : 0; }
unsigned int plat_ticks(void) { return pg_ticks(); }
void plat_delay(unsigned int ms) { pg_delay(ms); }
