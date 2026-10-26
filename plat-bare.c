// plat-bare.c -- c4sdl's platform with no operating system under it, or
// with C4DOS: the program is the machine's (libc's bare.c), and draws on
// libjs's display itself (plat-gui.c). A --timedemo runs without the
// display, as everywhere: its frames are checksummed, not shown.
//
// Under grin, C4DOS's graphical shell, the display is grin's, and grin
// publishes a table in C4DOS's slot 25 while it has it (c4's
// docs/grin-plan.md 3.6): then the game plays in a window of grin's
// instead, sending it its frames, and grin's loop runs in its polls.

#include "SDL.h"
#include "plat.h"
#define C4IX_SHARED 1
#include "gui.h"
#undef C4IX_SHARED

extern int c4sdl_timedemo;
extern int c4sdl_show;

// grin's table (c4's src/grin/gkit.h): its first eight words
struct grin_table {
    int magic, version, gen;
    void *env;
    void (*hello)(void);
    int  (*fwin_open)(char *title, int w, int h, int flags);
    void (*fwin_frame)(char *pixels, int w, int h, int pitch, int *pal);
    int  (*fwin_poll)(int *ev);
};
extern int *__c4dos_api;                // bare.c's: C4DOS's table, 0 on the bare machine

static struct grin_table *grin(void) {
    struct grin_table *t;
    if (!__c4dos_api || __c4dos_api[0] != 4404292 || __c4dos_api[5] < 4) return 0;
    t = (struct grin_table *)__c4dos_api[25];
    return t && t->magic == 1196575054 && t->version >= 2 ? t : 0;
}

static int shown;
static struct grin_table *g;            // grin's window, or 0: the display
static int gw, gh, gclosed;
static int gpal[256];                   // 0x00RRGGBB

int plat_init(void) { return 0; }
void plat_quit(void) { pg_close(); }

void plat_open(const char *title, int w, int h) {
    if (c4sdl_timedemo && !c4sdl_show) return;
    if ((g = grin())) {
        if (g->fwin_open((char *)title, w, h, 0)) { gw = w; gh = h; }
        else g = 0;
        return;
    }
    shown = pg_open(title, w, h);
}

void plat_present(unsigned char *pixels, int w, int h, int pitch, SDL_Color *pal) {
    int i;
    if (g) {
        if (gclosed || w != gw || h != gh) return;
        for (i = 0; i < 256; i++) gpal[i] = (pal[i].r << 16) | (pal[i].g << 8) | pal[i].b;
        g->fwin_frame((char *)pixels, w, h, pitch, gpal);
        return;
    }
    if (shown) pg_present(pixels, w, h, pitch, pal);
}

int plat_event(int *kind, int *sc, int *ch) {
    int ev[4], k;
    if (!g) return shown ? pg_event(kind, sc, ch) : 0;
    if (gclosed) return 0;
    while ((k = g->fwin_poll(ev)) > 0) {
        if (ev[0] != GUI_EV_KEYDOWN && ev[0] != GUI_EV_KEYUP) continue;
        *kind = ev[0] == GUI_EV_KEYDOWN ? PLAT_DOWN : PLAT_UP;
        *sc = plat_scancode(ev[1]);
        *ch = ev[2];
        if (*sc == SDL_SCANCODE_UNKNOWN && *ch == 0) continue;
        return 1;
    }
    // the window closed: the game ends, as when it is quit
    if (k < 0) { gclosed = 1; *kind = PLAT_QUIT; *sc = 0; *ch = 0; return 1; }
    return 0;
}

unsigned int plat_ticks(void) { return pg_ticks(); }
void plat_delay(unsigned int ms) { pg_delay(ms); }
