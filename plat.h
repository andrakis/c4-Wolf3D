// plat.h -- what c4sdl.c needs from the machine it runs on.
// plat-host.c (gcc, headless) and plat-c4ix.c (a C4IX desktop window).

#ifndef C4SDL_PLAT_H
#define C4SDL_PLAT_H 1

enum { PLAT_NONE = 0, PLAT_DOWN = 1, PLAT_UP = 2, PLAT_QUIT = 3 };

int  plat_init(void);
void plat_quit(void);
void plat_open(const char *title, int w, int h);
// Show W x H 8-bit pixels through PAL (256 SDL_Colors: r, g, b, a).
void plat_present(unsigned char *pixels, int w, int h, int pitch, SDL_Color *pal);
// The next key event: KIND is PLAT_DOWN, PLAT_UP or PLAT_QUIT, SC an
// SDL scancode, CH its character or 0. 0 when there is none.
int  plat_event(int *kind, int *sc, int *ch);
unsigned int plat_ticks(void);           // milliseconds
void plat_delay(unsigned int ms);

// The sound card, if there is one: libjs's (libjs/guest/snd.h). An OPL
// write is stamped with the stream's time, which plat_snd_wait moves on
// in samples at RATE; plat_snd_ahead is how far ahead it is, in
// microseconds. A sample plays now: unsigned 8-bit, vol left | right << 8.
int  plat_snd_open(int rate);            // 0: there is none
void plat_snd_close(void);
void plat_snd_opl(int reg, int val);
void plat_snd_wait(int samples);
int  plat_snd_ahead(void);
void plat_snd_play(int ch, unsigned char *data, int len, int rate, int vol);
void plat_snd_stop(int ch);              // -1: all
void plat_snd_volume(int ch, int vol);
int  plat_snd_busy(void);                // a bit per channel still playing

#endif
