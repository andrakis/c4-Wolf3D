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
int c4sdl_sound;               // --sound: a timedemo plays sound too
int c4sdl_show;                // --show: a timedemo on the display or window too
int c4sdl_direct = 1;          // the display itself; --window: a window (plat-c4ix.c)
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
    c4sdl_sound_service();
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
    c4sdl_sound_service();
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
    c4sdl_sound_service();
    if (c4sdl_timedemo) { c4sdl_vclock += ms; return; }
    plat_delay(ms);
}

// ---- the mixer, over the platform's sound card ----
//
// Eight channels of the game's own samples: SDL_mixer would convert them
// to the device's rate, and id_sd.c did, in floating point, which took
// seconds; the card plays them at their own. Which channels are still
// playing is the card's to say, so Mix_ChannelFinished's callback is
// called when one is seen to have stopped: here, in the service, and
// when a sound is halted or replaced, as SDL_mixer calls it.
//
// The music is id_sd.c's SDL_IMFMusicPlayer, which SDL_mixer would call
// from its audio thread for each buffer. Here it is called from the game's
// own loop -- the event pump, SDL_Delay, each frame -- until the card's
// stream is MUSIC_AHEAD ahead: its OPL writes are stamped with stream
// time (YM3812UpdateOne only moves the stream on), so it plays evenly
// however unevenly the game calls it.
enum { RATE = 44100, MUSIC_AHEAD = 100000, MUSIC_BLOCK = 441, MUSIC_MAX = 64 };
static int snd_on;
static void (*music_fn)(void *, Uint8 *, int);
static void *music_arg;
static void (*finished_fn)(int);
static int music_busy;                    // in music_fn: no service from inside it
static int playing;                       // channels playing, as last seen
static int reserved;                      // below this, only by number
static int group_of[MIX_CHANNELS];
static unsigned int started[MIX_CHANNELS];
static unsigned int nstarted;
static int chan_vol[MIX_CHANNELS];
static Uint8 music_buf[1764];             // MUSIC_BLOCK stereo 16-bit samples

// channels that were playing and are not now: their callbacks
static void finished(int stopped) {
    int i;
    playing &= ~stopped;
    if (!finished_fn) return;
    for (i = 0; i < MIX_CHANNELS; i++)
        if ((stopped >> i) & 1) finished_fn(i);
}

void c4sdl_sound_service(void) {
    int n;
    if (!snd_on) return;
    if (music_fn && !music_busy) {
        music_busy = 1;
        n = 0;
        while (n < MUSIC_MAX && plat_snd_ahead() < MUSIC_AHEAD) {
            music_fn(music_arg, music_buf, MUSIC_BLOCK * 4);
            ++n;
        }
        music_busy = 0;
    }
    finished(playing & ~plat_snd_busy());
}

int Mix_OpenAudioDevice(int frequency, Uint16 format, int channels, int chunksize, const char *device, int changes) {
    int i;
    // a timedemo is silent unless asked, so its frames are the gcc build's
    if (c4sdl_timedemo && !c4sdl_sound) return -1;
    if (!plat_snd_open(RATE)) return -1;
    for (i = 0; i < MIX_CHANNELS; i++) { group_of[i] = -1; chan_vol[i] = 0xffff; }
    snd_on = 1;
    return 0;
}
void Mix_CloseAudio(void) {
    if (!snd_on) return;
    plat_snd_close();
    snd_on = 0;
    music_fn = 0;
}
int Mix_QuerySpec(int *frequency, Uint16 *format, int *channels) {
    if (frequency) *frequency = RATE;
    if (format) *format = AUDIO_S16;
    if (channels) *channels = 2;
    return snd_on;
}
const char *Mix_GetError(void) { return snd_on ? "c4sdl: no free channel" : "no sound device"; }
int Mix_ReserveChannels(int num) { reserved = num; return num; }
int Mix_GroupChannels(int from, int to, int tag) {
    int i;
    for (i = from; i <= to && i < MIX_CHANNELS; i++) group_of[i] = tag;
    return to - from + 1;
}
int Mix_GroupAvailable(int tag) {
    int i, b;
    b = snd_on ? plat_snd_busy() : 0;
    for (i = 0; i < MIX_CHANNELS; i++)
        if ((tag == -1 || group_of[i] == tag) && !((b >> i) & 1)) return i;
    return -1;
}
int Mix_GroupOldest(int tag) {
    int i, b, best;
    b = snd_on ? plat_snd_busy() : 0;
    best = -1;
    for (i = 0; i < MIX_CHANNELS; i++)
        if ((tag == -1 || group_of[i] == tag) && ((b >> i) & 1) &&
            (best < 0 || started[i] < started[best])) best = i;
    return best;
}
int Mix_HaltChannel(int channel) {
    if (!snd_on) return 0;
    plat_snd_stop(channel);
    finished(channel < 0 ? playing : playing & (1 << channel));
    return 0;
}
int Mix_SetPanning(int channel, Uint8 left, Uint8 right) {
    if (channel < 0 || channel >= MIX_CHANNELS) return 0;
    chan_vol[channel] = left | (right << 8);
    // a sound that moves is panned as it plays
    if (snd_on && ((plat_snd_busy() >> channel) & 1)) plat_snd_volume(channel, chan_vol[channel]);
    return 1;
}
int Mix_PlayChannel(int channel, Mix_Chunk *chunk, int loops) {
    int b;
    if (!snd_on || !chunk || !chunk->alen) return -1;
    b = plat_snd_busy();
    if (channel < 0) {
        channel = reserved;
        while (channel < MIX_CHANNELS && ((b >> channel) & 1)) ++channel;
        if (channel >= MIX_CHANNELS) return -1;
    }
    if (channel >= MIX_CHANNELS) return -1;
    // the sound it replaces is finished, before this one starts
    finished(playing & (1 << channel));
    plat_snd_play(channel, chunk->abuf, chunk->alen, chunk->rate, chan_vol[channel]);
    started[channel] = ++nstarted;
    playing |= 1 << channel;
    return channel;
}
Mix_Chunk *c4sdl_rawchunk(Uint8 *samples, int len, int rate) {
    Mix_Chunk *c;
    c = (Mix_Chunk *)malloc(sizeof(Mix_Chunk));
    if (!c) return NULL;
    c->allocated = 0;
    c->abuf = samples;
    c->alen = len;
    c->volume = 128;
    c->rate = rate;
    return c;
}
void Mix_FreeChunk(Mix_Chunk *chunk) { if (chunk) free(chunk); }
Mix_Chunk *Mix_LoadWAV_RW(SDL_RWops *src, int freesrc) { return NULL; }
void Mix_HookMusic(void (*fn)(void *, Uint8 *, int), void *arg) { music_fn = fn; music_arg = arg; }
void Mix_ChannelFinished(void (*fn)(int)) { finished_fn = fn; }
// the PC speaker's mixer: not here, so the PC speaker is silent
int Mix_SetPostMix(void (*fn)(void *, Uint8 *, int), void *arg) { return 0; }
SDL_RWops *SDL_RWFromMem(void *mem, int size) { return NULL; }

// ---- the AdLib: the sound card's OPL ----
int YM3812Init(int num, int clock, int rate) { return snd_on ? 0 : 1; }
void YM3812Write(int chip, int reg, int val) { if (snd_on) plat_snd_opl(reg, val); }
// the music player's samples: the card makes them, so the stream moves on
void YM3812UpdateOne(int chip, short *buf, int length) { if (snd_on) plat_snd_wait(length); }
