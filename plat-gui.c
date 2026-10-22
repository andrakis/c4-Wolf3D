// plat-gui.c -- c4sdl's platform drawn on libjs's display itself
// (libjs/guest/gui.h), as raycast -G does: what the bare machine and C4DOS
// run (plat-bare.c), and C4IX with --direct (plat-c4ix.c). And what every
// target shares: the sound card (libjs/guest/snd.h), and a browser
// keyCode as an SDL scancode.
//
// The game's 320x200 frames go to a 640x480 display: 2x across and 2.4x
// down, the 4:3 shape the game was drawn for on a VGA monitor. Each is
// sent as GUI_BLIT8 -- the 8-bit pixels and the palette, read by the host
// when the bell rings, so the game draws its next frame at once. Keys come
// from the display's event ring; time is the machine's (TIME), and a delay
// is a USLP, which on C4IX is a sleep of this task alone.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL.h"
#include "plat.h"
#include "gui.h"
#include "snd.h"

enum { PG_W = 640, PG_H = 480, PG_RING = 65536 };

static int pg_on;                       // the display is ours
static int *pg_ring;
static int pg_pal[256];                 // 0x00RRGGBB
static int pg_w, pg_h;

// The display, or 0 (with why) where there is none
int pg_open(const char *title, int w, int h) {
    if (!gui_present()) { printf("wolf3d: this machine has no display\n"); return 0; }
    if (!(pg_ring = (int *)malloc(PG_RING))) return 0;
    if (!gui_attach(pg_ring, PG_RING, PG_W, PG_H)) return 0;
    gui_events(GUI_M_KEYS | GUI_M_WINDOW);
    gui_cursor(GUI_PTR_NONE);
    gui_clear(0x000000);
    gui_show();
    pg_w = w; pg_h = h;
    pg_on = 1;
    return 1;
}

void pg_close(void) {
    if (!pg_on) return;
    gui_cursor(GUI_PTR_ARROW);
    gui_detach();
    pg_on = 0;
}

void pg_present(unsigned char *pixels, int w, int h, int pitch, SDL_Color *pal) {
    int i;
    if (!pg_on || w != pg_w || h != pg_h) return;
    for (i = 0; i < 256; i++) pg_pal[i] = (pal[i].r << 16) | (pal[i].g << 8) | pal[i].b;
    gui_blit8(0, 0, w, h, (char *)pixels, pitch, pg_pal, PG_W, PG_H);
    gui_show();
}

int pg_event(int *kind, int *sc, int *ch) {
    int ev[4];
    if (!pg_on) return 0;
    while (gui_poll(ev)) {
        if (ev[0] != GUI_EV_KEYDOWN && ev[0] != GUI_EV_KEYUP) continue;
        *kind = ev[0] == GUI_EV_KEYDOWN ? PLAT_DOWN : PLAT_UP;
        *sc = plat_scancode(ev[1]);
        *ch = ev[2];
        if (*sc == SDL_SCANCODE_UNKNOWN && *ch == 0) continue;
        return 1;
    }
    return 0;
}

unsigned int pg_ticks(void) { return __time(); }
void pg_delay(unsigned int ms) { if (ms) __c4_usleep(ms * 1000); }

// A browser keyCode as an SDL scancode
int plat_scancode(int code) {
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

// ---- the sound card: libjs's, when the machine has one ----
int plat_snd_open(int rate) {
    if (!snd_present()) return 0;
    snd_reset();
    snd_clock(rate);
    return 1;
}
void plat_snd_close(void) { if (snd_present()) snd_reset(); }
void plat_snd_opl(int reg, int val) { snd_opl(reg, val); }
void plat_snd_wait(int samples) { snd_wait(samples); }
int  plat_snd_ahead(void) { return snd_ahead(); }
void plat_snd_play(int ch, unsigned char *data, int len, int rate, int vol) { snd_play(ch, (char *)data, len, rate, vol); }
void plat_snd_stop(int ch) { snd_stop(ch); }
void plat_snd_volume(int ch, int vol) { snd_volume(ch, vol); }
int  plat_snd_busy(void) { return snd_busy(); }
