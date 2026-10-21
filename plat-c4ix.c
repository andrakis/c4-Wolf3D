// plat-c4ix.c -- c4sdl's platform on C4IX: the Terminal window it runs
// in becomes its canvas (window.h), showing 8-bit frames the desktop has
// the host scale and colour (win_fb, GUI_BLIT8).
//
// Two buffers: the desktop reads the one it was given when it next
// draws, and says so (WIN_FRAME); until then a newer frame waits in the
// other, and only the newest is sent. The game never waits on the
// window, and a frame is never shown half drawn.
//
// --timedemo runs without a window (the frames are checksummed, not
// shown), unless WOLF_WINDOW is set.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "SDL.h"
#include "plat.h"
#include "window.h"

extern int c4sdl_timedemo;

static int windowed;
static unsigned char *fbuf[2];          // the frames: [sent] the desktop's
static int *fpal[2];                    // their colours, 0x00RRGGBB
static int sent;                        // the buffer the desktop has
static int waiting;                     // it has not yet drawn it
static int pending;                     // the other holds a newer frame
static int fw, fh;
static int closed;

int plat_init(void) {
    windowed = !c4sdl_timedemo || getenv("WOLF_WINDOW") != NULL;
    return 0;
}

void plat_quit(void) {
    if (windowed && !closed) { win_close(); closed = 1; }
}

void plat_open(const char *title, int w, int h) {
    int i;
    if (!windowed) return;
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
    if (!windowed || w != fw || h != fh) return;
    // the buffer the desktop is not reading
    b = waiting ? 1 - sent : sent;
    for (y = 0; y < h; y++) memcpy(fbuf[b] + y * w, pixels + y * pitch, w);
    for (i = 0; i < 256; i++) fpal[b][i] = (pal[i].r << 16) | (pal[i].g << 8) | pal[i].b;
    if (waiting) pending = 1;
    else send_frame(b);
}

// A browser keyCode as an SDL scancode
static int scancode(int code) {
    if (code >= 'A' && code <= 'Z') return SDL_SCANCODE_A + code - 'A';
    if (code >= '1' && code <= '9') return SDL_SCANCODE_1 + code - '1';
    if (code == '0') return SDL_SCANCODE_0;
    if (code >= 112 && code <= 123) return SDL_SCANCODE_F1 + code - 112;
    switch (code) {
        case 13: return SDL_SCANCODE_RETURN;
        case 27: return SDL_SCANCODE_ESCAPE;
        case 8: return SDL_SCANCODE_BACKSPACE;
        case 9: return SDL_SCANCODE_TAB;
        case 32: return SDL_SCANCODE_SPACE;
        case 189: case 173: return SDL_SCANCODE_MINUS;
        case 187: case 61: return SDL_SCANCODE_EQUALS;
        case 219: return SDL_SCANCODE_LEFTBRACKET;
        case 221: return SDL_SCANCODE_RIGHTBRACKET;
        case 220: return SDL_SCANCODE_BACKSLASH;
        case 186: case 59: return SDL_SCANCODE_SEMICOLON;
        case 222: return SDL_SCANCODE_APOSTROPHE;
        case 192: return SDL_SCANCODE_GRAVE;
        case 188: return SDL_SCANCODE_COMMA;
        case 190: return SDL_SCANCODE_PERIOD;
        case 191: return SDL_SCANCODE_SLASH;
        case 20: return SDL_SCANCODE_CAPSLOCK;
        case 145: return SDL_SCANCODE_SCROLLLOCK;
        case 19: return SDL_SCANCODE_PAUSE;
        case 45: return SDL_SCANCODE_INSERT;
        case 46: return SDL_SCANCODE_DELETE;
        case 36: return SDL_SCANCODE_HOME;
        case 35: return SDL_SCANCODE_END;
        case 33: return SDL_SCANCODE_PAGEUP;
        case 34: return SDL_SCANCODE_PAGEDOWN;
        case 37: return SDL_SCANCODE_LEFT;
        case 38: return SDL_SCANCODE_UP;
        case 39: return SDL_SCANCODE_RIGHT;
        case 40: return SDL_SCANCODE_DOWN;
        case 16: return SDL_SCANCODE_LSHIFT;
        case 17: return SDL_SCANCODE_LCTRL;
        case 18: return SDL_SCANCODE_LALT;
    }
    return SDL_SCANCODE_UNKNOWN;
}

int plat_event(int *kind, int *sc, int *ch) {
    int ev[3], k;
    if (!windowed || closed) return 0;
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
            *sc = scancode(ev[0]);
            *ch = ev[1];
            if (*sc == SDL_SCANCODE_UNKNOWN && *ch == 0) continue;
            return 1;
        }
    }
}

unsigned int plat_ticks(void) { return clock(); }

void plat_delay(unsigned int ms) {

    win_sleep(ms);
}
