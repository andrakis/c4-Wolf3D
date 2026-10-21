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

#endif
