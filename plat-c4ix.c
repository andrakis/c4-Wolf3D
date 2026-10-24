// plat-c4ix.c -- c4sdl's platform on C4IX. The game takes libjs's whole
// display (plat-gui.c), as raycast -G does: from the desktop, which gives
// way until the game ends (include/display.h), or from a console with no
// desktop on it. With --window, or when a program that does not give way
// has the display, the Terminal window it runs in becomes its canvas
// instead (window.h), showing 8-bit frames the desktop has the host scale
// and colour (win_fb, GUI_BLIT8). The sound card is plat-gui.c's either
// way.
//
// Two buffers: the desktop reads the one it was given when it next
// draws, and says so (WIN_FRAME); until then a newer frame waits in the
// other, and only the newest is sent. The game never waits on the
// window, and a frame is never shown half drawn.
//
// --timedemo runs without a window (the frames are checksummed, not
// shown), unless --show is given or WOLF_WINDOW is set.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "SDL.h"
#include "plat.h"
#include "window.h"
#include "display.h"

extern int c4sdl_timedemo;
extern int c4sdl_show;
int unice(int id, int nice);            // libc4ix (c4ix_user.h): nice, sched.c

static int windowed;
static unsigned char *fbuf[2];          // the frames: [sent] the desktop's
static int *fpal[2];                    // their colours, 0x00RRGGBB
static int sent;                        // the buffer the desktop has
static int waiting;                     // it has not yet drawn it
static int pending;                     // the other holds a newer frame
static int fw, fh;
static int closed;

int plat_init(void) {
    windowed = !c4sdl_timedemo || c4sdl_show || getenv("WOLF_WINDOW") != NULL;
    // a game asks to be served sooner and longer than the rest (nice, sched.c)
    if (windowed) unice(0, -5);
    return 0;
}

void plat_quit(void) {
    if (c4sdl_direct) { pg_close(); udisplay_give(); return; }
    if (windowed && !closed) { win_close(); closed = 1; }
}

void plat_open(const char *title, int w, int h) {
    int i;
    if (!windowed) return;
    // the display, once the desktop has given way; or the window
    if (c4sdl_direct && !udisplay_take(0)) {
        printf("wolf3d: a program has the display; playing in the window\n");
        c4sdl_direct = 0;
    }
    if (c4sdl_direct) {
        if (!pg_open(title, w, h)) { windowed = 0; udisplay_give(); }
        return;
    }
    fw = w; fh = h;
    for (i = 0; i < 2; i++) {
        fbuf[i] = (unsigned char *)calloc(w * h, 1);
        fpal[i] = (int *)calloc(256, sizeof(int));
    }
    if (!win_open((char *)title)) windowed = 0;
}

static void send_frame(int b) {
    win_fb((char *)fbuf[b], fw, fh, fw, fpal[b]);
    win_present();
    sent = b;
    waiting = 1;
}

void plat_present(unsigned char *pixels, int w, int h, int pitch, SDL_Color *pal) {
    int b, y, i;
    if (!windowed) return;
    if (c4sdl_direct) { pg_present(pixels, w, h, pitch, pal); return; }
    if (w != fw || h != fh) return;
    // the buffer the desktop is not reading
    b = waiting ? 1 - sent : sent;
    for (y = 0; y < h; y++) memcpy(fbuf[b] + y * w, pixels + y * pitch, w);
    for (i = 0; i < 256; i++) fpal[b][i] = (pal[i].r << 16) | (pal[i].g << 8) | pal[i].b;
    if (waiting) pending = 1;
    else send_frame(b);
}

int plat_event(int *kind, int *sc, int *ch) {
    int ev[3], k;
    if (!windowed || closed) return 0;
    if (c4sdl_direct) return pg_event(kind, sc, ch);
    while (1) {
        k = win_poll(ev);
        if (k == WIN_NONE) return 0;
        if (k < 0) { closed = 1; *kind = PLAT_QUIT; *sc = 0; *ch = 0; return 1; }
        if (k == WIN_FRAME) {
            // the desktop has drawn what it had: the newer frame, if any
            waiting = 0;
            if (pending) { pending = 0; send_frame(1 - sent); }
            continue;
        }
        if (k == WIN_KEY || k == WIN_KEYUP) {
            *kind = k == WIN_KEY ? PLAT_DOWN : PLAT_UP;
            *sc = plat_scancode(ev[0]);
            *ch = ev[1];
            if (*sc == SDL_SCANCODE_UNKNOWN && *ch == 0) continue;
            return 1;
        }
    }
}

unsigned int plat_ticks(void) { return clock(); }

void plat_delay(unsigned int ms) {
    if (c4sdl_direct) { pg_delay(ms); return; }
    win_sleep(ms);
}
