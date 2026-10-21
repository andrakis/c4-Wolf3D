// c4sdl.c -- SDL.h's functions, over a platform layer of five calls
// (plat.h): plat-host.c for gcc on a host, plat-c4ix.c for a C4IX
// desktop window.
//
// It also keeps the timedemo's books: with c4sdl_timedemo set, time is
// virtual (each SDL_GetTicks is a millisecond later, SDL_Delay adds its
// wait), so a demo plays the same frames on every machine, and each
// frame presented is folded into a checksum that the gcc and c4lc
// builds must agree on.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL.h"
#include "SDL_mixer.h"
#include "plat.h"

int c4sdl_timedemo;            // virtual time, and frames counted
unsigned int c4sdl_frames;
unsigned int c4sdl_sum;
static unsigned int c4sdl_vclock;

// ---- start-up, errors, hints ----
int SDL_Init(Uint32 flags) { return plat_init(); }
void SDL_Quit(void) { plat_quit(); }
const char *SDL_GetError(void) { return "c4sdl"; }
int SDL_SetHint(const char *name, const char *value) { return 1; }

int SDL_ShowSimpleMessageBox(Uint32 flags, const char *title, const char *msg, SDL_Window *w) {
    fprintf(stderr, "%s: %s\n", title, msg);
    return 0;
}

// ---- windows, renderers, textures: tokens ----
static SDL_Window the_window;
static SDL_Renderer the_renderer;
static SDL_Texture the_texture;
SDL_Window *SDL_CreateWindow(const char *title, int x, int y, int w, int h, Uint32 flags) {
    plat_open(title, w, h);
    return &the_window;
}
void SDL_DestroyWindow(SDL_Window *w) { }
void SDL_SetWindowGrab(SDL_Window *w, SDL_bool grab) { }
SDL_Renderer *SDL_CreateRenderer(SDL_Window *w, int index, Uint32 flags) { return &the_renderer; }
void SDL_DestroyRenderer(SDL_Renderer *r) { }
int SDL_SetRenderDrawBlendMode(SDL_Renderer *r, int mode) { return 0; }
SDL_Texture *SDL_CreateTexture(SDL_Renderer *r, Uint32 format, int access, int w, int h) { return &the_texture; }
void SDL_DestroyTexture(SDL_Texture *t) { }
int SDL_ShowCursor(int toggle) { return 0; }
int SDL_SetRelativeMouseMode(SDL_bool on) { return 0; }
void SDL_WarpMouseInWindow(SDL_Window *w, int x, int y) { }

// ---- surfaces: 8-bit ones are all the port draws on ----
SDL_bool SDL_PixelFormatEnumToMasks(Uint32 format, int *bpp, Uint32 *r, Uint32 *g, Uint32 *b, Uint32 *a) {
    *bpp = 8; *r = 0; *g = 0; *b = 0; *a = 0;
    return 1;
}

SDL_Surface *SDL_CreateRGBSurface(Uint32 flags, int w, int h, int depth, Uint32 r, Uint32 g, Uint32 b, Uint32 a) {
    SDL_Surface *s;
    SDL_PixelFormat *f;
    SDL_Palette *p;
    int i;
    s = (SDL_Surface *)calloc(1, sizeof(SDL_Surface));
    f = (SDL_PixelFormat *)calloc(1, sizeof(SDL_PixelFormat));
    p = (SDL_Palette *)calloc(1, sizeof(SDL_Palette));
    p->ncolors = 256;
    p->colors = (SDL_Color *)calloc(256, sizeof(SDL_Color));
    i = 0;
    while (i < 256) { p->colors[i].a = 255; ++i; }
    f->palette = p;
    f->BitsPerPixel = 8;
    f->BytesPerPixel = 1;
    s->format = f;
    s->w = w;
    s->h = h;
    s->pitch = w;
    s->pixels = calloc(w * h, 1);
    if (!s->pixels) return NULL;
    return s;
}

void SDL_FreeSurface(SDL_Surface *s) {
    if (!s) return;
    free(s->pixels);
    free(s->format->palette->colors);
    free(s->format->palette);
    free(s->format);
    free(s);
}

int SDL_SetPaletteColors(SDL_Palette *p, const SDL_Color *colors, int first, int n) {
    memcpy(p->colors + first, colors, n * sizeof(SDL_Color));
    return 0;
}

int SDL_BlitSurface(SDL_Surface *src, const SDL_Rect *sr, SDL_Surface *dst, SDL_Rect *dr) {
    int y, w, h;
    Uint8 *s, *d;
    w = src->w < dst->w ? src->w : dst->w;
    h = src->h < dst->h ? src->h : dst->h;
    s = (Uint8 *)src->pixels;
    d = (Uint8 *)dst->pixels;
    y = 0;
    while (y < h) {
        memcpy(d, s, w);
        s += src->pitch;
        d += dst->pitch;
        ++y;
    }
    return 0;
}

int SDL_FillRect(SDL_Surface *dst, const SDL_Rect *r, Uint32 color) {
    int y;
    Uint8 *d;
    if (!r) { memset(dst->pixels, color, dst->pitch * dst->h); return 0; }
    d = (Uint8 *)dst->pixels + r->y * dst->pitch + r->x;
    y = 0;
    while (y < r->h) { memset(d, color, r->w); d += dst->pitch; ++y; }
    return 0;
}

int SDL_LockSurface(SDL_Surface *s) { return 0; }
void SDL_UnlockSurface(SDL_Surface *s) { }
Uint32 SDL_MapRGBA(const SDL_PixelFormat *f, Uint8 r, Uint8 g, Uint8 b, Uint8 a) { return 0; }
int SDL_SaveBMP(SDL_Surface *s, const char *file) { return -1; }

// ---- the frame ----
void c4sdl_present(Uint8 *pixels, int w, int h, int pitch, SDL_Color *pal) {
    unsigned int sum;
    int x, y;
    unsigned int *words;
    if (c4sdl_timedemo) {
        // the frame and its colours, as the window would show them: four
        // pixels at a time, a little-endian word (the rows are whole words)
        sum = c4sdl_sum;
        y = 0;
        while (y < h) {
            words = (unsigned int *)(pixels + y * pitch);
            x = 0;
            while (x < w / 4) {
                sum = (sum * 31 + words[x]) & 0xFFFFFFFF;
                ++x;
            }
            ++y;
        }
        x = 0;
        while (x < 256) {
            sum = (sum * 31 + pal[x].r * 65536 + pal[x].g * 256 + pal[x].b) & 0xFFFFFFFF;
            ++x;
        }
        c4sdl_sum = sum;
        ++c4sdl_frames;
        if (c4sdl_timedemo > 1)
            printf("frame %u %08x\n", c4sdl_frames, sum);
    }
    plat_present(pixels, w, h, pitch, pal);
}

// ---- events ----
enum { QLEN = 64 };
static SDL_Event queue[QLEN];
static int qhead, qtail;
static int modstate;

static void put(SDL_Event *e) {
    int next;
    next = (qtail + 1) % QLEN;
    if (next == qhead) return;            // full: drop it
    queue[qtail] = *e;
    qtail = next;
}

// Turn what the platform has into events
static void pump(void) {
    int kind, sc, ch;
    SDL_Event e;
    while (plat_event(&kind, &sc, &ch)) {
        memset(&e, 0, sizeof(e));
        if (kind == PLAT_QUIT) { e.type = SDL_QUIT; put(&e); continue; }
        if (sc == SDL_SCANCODE_LSHIFT || sc == SDL_SCANCODE_RSHIFT) {
            if (kind == PLAT_DOWN) modstate |= KMOD_LSHIFT; else modstate &= ~KMOD_LSHIFT;
        }
        e.type = kind == PLAT_DOWN ? SDL_KEYDOWN : SDL_KEYUP;
        e.key.keysym.scancode = sc;
        e.key.keysym.sym = ch;
        e.key.keysym.mod = modstate;
        put(&e);
        if (kind == PLAT_DOWN && ch >= 32 && ch < 127) {
            memset(&e, 0, sizeof(e));
            e.type = SDL_TEXTINPUT;
            e.text.text[0] = ch;
            put(&e);
        }
    }
}

int SDL_PollEvent(SDL_Event *e) {
    pump();
    if (qhead == qtail) return 0;
    if (e) *e = queue[qhead];
    qhead = (qhead + 1) % QLEN;
    return 1;
}

int SDL_WaitEvent(SDL_Event *e) {
    pump();
    while (qhead == qtail) {
        SDL_Delay(5);
        pump();
    }
    if (e) return SDL_PollEvent(e);
    return 1;
}

Uint8 SDL_EventState(Uint32 type, int state) { return 0; }
SDL_Keymod SDL_GetModState(void) { return modstate; }
Uint32 SDL_GetMouseState(int *x, int *y) { if (x) *x = 0; if (y) *y = 0; return 0; }
Uint32 SDL_GetRelativeMouseState(int *x, int *y) { if (x) *x = 0; if (y) *y = 0; return 0; }

// ---- no joysticks ----
int SDL_NumJoysticks(void) { return 0; }
SDL_Joystick *SDL_JoystickOpen(int index) { return NULL; }
void SDL_JoystickClose(SDL_Joystick *j) { }
void SDL_JoystickUpdate(void) { }
int SDL_JoystickNumButtons(SDL_Joystick *j) { return 0; }
int SDL_JoystickNumHats(SDL_Joystick *j) { return 0; }
Sint16 SDL_JoystickGetAxis(SDL_Joystick *j, int axis) { return 0; }
Uint8 SDL_JoystickGetButton(SDL_Joystick *j, int button) { return 0; }
Uint8 SDL_JoystickGetHat(SDL_Joystick *j, int hat) { return 0; }

// ---- time ----
Uint32 SDL_GetTicks(void) {
    if (c4sdl_timedemo) { ++c4sdl_vclock; return c4sdl_vclock; }
    return plat_ticks();
}

void SDL_Delay(Uint32 ms) {
    if (c4sdl_timedemo) { c4sdl_vclock += ms; return; }
    plat_delay(ms);
}

// ---- no audio device yet ----
int Mix_OpenAudioDevice(int frequency, Uint16 format, int channels, int chunksize, const char *device, int changes) { return -1; }
int Mix_QuerySpec(int *frequency, Uint16 *format, int *channels) { return 0; }
const char *Mix_GetError(void) { return "no sound device"; }
int Mix_ReserveChannels(int num) { return 0; }
int Mix_GroupChannels(int from, int to, int tag) { return 0; }
int Mix_GroupAvailable(int tag) { return -1; }
int Mix_GroupOldest(int tag) { return -1; }
int Mix_HaltChannel(int channel) { return 0; }
int Mix_SetPanning(int channel, Uint8 left, Uint8 right) { return 0; }
int Mix_PlayChannel(int channel, Mix_Chunk *chunk, int loops) { return -1; }
void Mix_FreeChunk(Mix_Chunk *chunk) { }
Mix_Chunk *Mix_LoadWAV_RW(SDL_RWops *src, int freesrc) { return NULL; }
void Mix_HookMusic(void (*fn)(void *, Uint8 *, int), void *arg) { }
void Mix_ChannelFinished(void (*fn)(int)) { }
int Mix_SetPostMix(void (*fn)(void *, Uint8 *, int), void *arg) { return 0; }
SDL_RWops *SDL_RWFromMem(void *mem, int size) { return NULL; }

// ---- the AdLib, for when there is one (c4opl.h) ----
int YM3812Init(int num, int clock, int rate) { return 1; }
void YM3812Write(int chip, int reg, int val) { }
void YM3812UpdateOne(int chip, short *buf, int length) { memset(buf, 0, length * 4); }
