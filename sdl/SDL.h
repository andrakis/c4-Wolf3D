// SDL.h -- the part of SDL 2 that Wolf4SDL uses, and no more.
//
// The port draws into one 8-bit surface and hands it, with its palette,
// to a platform layer (c4sdl.c, which calls the plat_* functions of
// plat-host.c or plat-c4ix.c). Windows, renderers and textures are
// tokens; there is no audio device, no joystick and no mouse.
//
// Written for gcc and for c4lc alike.

#ifndef C4SDL_H
#define C4SDL_H 1

#include <stdint.h>

typedef uint8_t  Uint8;
typedef int8_t   Sint8;
typedef uint16_t Uint16;
typedef int16_t  Sint16;
typedef uint32_t Uint32;
typedef int32_t  Sint32;
typedef int      SDL_bool;

#define SDL_ALPHA_OPAQUE 255

typedef struct SDL_Color { Uint8 r, g, b, a; } SDL_Color;
typedef struct SDL_Palette { int ncolors; SDL_Color *colors; } SDL_Palette;
typedef struct SDL_PixelFormat { SDL_Palette *palette; int BitsPerPixel; int BytesPerPixel; } SDL_PixelFormat;
typedef struct SDL_Rect { int x, y, w, h; } SDL_Rect;
typedef struct SDL_Surface {
    int flags;
    SDL_PixelFormat *format;
    int w, h, pitch;
    void *pixels;
} SDL_Surface;

typedef struct SDL_Window   { int id; } SDL_Window;
typedef struct SDL_Renderer { int id; } SDL_Renderer;
typedef struct SDL_Texture  { int id; } SDL_Texture;
typedef struct SDL_Joystick { int id; } SDL_Joystick;

// ---- scancodes: SDL 2's numbers, the USB HID usage codes ----
typedef int SDL_Scancode;
enum {
    SDL_SCANCODE_UNKNOWN = 0,
    SDL_SCANCODE_A = 4, SDL_SCANCODE_B, SDL_SCANCODE_C, SDL_SCANCODE_D, SDL_SCANCODE_E,
    SDL_SCANCODE_F, SDL_SCANCODE_G, SDL_SCANCODE_H, SDL_SCANCODE_I, SDL_SCANCODE_J,
    SDL_SCANCODE_K, SDL_SCANCODE_L, SDL_SCANCODE_M, SDL_SCANCODE_N, SDL_SCANCODE_O,
    SDL_SCANCODE_P, SDL_SCANCODE_Q, SDL_SCANCODE_R, SDL_SCANCODE_S, SDL_SCANCODE_T,
    SDL_SCANCODE_U, SDL_SCANCODE_V, SDL_SCANCODE_W, SDL_SCANCODE_X, SDL_SCANCODE_Y,
    SDL_SCANCODE_Z,
    SDL_SCANCODE_1 = 30, SDL_SCANCODE_2, SDL_SCANCODE_3, SDL_SCANCODE_4, SDL_SCANCODE_5,
    SDL_SCANCODE_6, SDL_SCANCODE_7, SDL_SCANCODE_8, SDL_SCANCODE_9, SDL_SCANCODE_0,
    SDL_SCANCODE_RETURN = 40, SDL_SCANCODE_ESCAPE = 41, SDL_SCANCODE_BACKSPACE = 42,
    SDL_SCANCODE_TAB = 43, SDL_SCANCODE_SPACE = 44, SDL_SCANCODE_MINUS = 45,
    SDL_SCANCODE_EQUALS = 46, SDL_SCANCODE_LEFTBRACKET = 47, SDL_SCANCODE_RIGHTBRACKET = 48,
    SDL_SCANCODE_BACKSLASH = 49, SDL_SCANCODE_SEMICOLON = 51, SDL_SCANCODE_APOSTROPHE = 52,
    SDL_SCANCODE_GRAVE = 53, SDL_SCANCODE_COMMA = 54, SDL_SCANCODE_PERIOD = 55,
    SDL_SCANCODE_SLASH = 56, SDL_SCANCODE_CAPSLOCK = 57,
    SDL_SCANCODE_F1 = 58, SDL_SCANCODE_F2, SDL_SCANCODE_F3, SDL_SCANCODE_F4, SDL_SCANCODE_F5,
    SDL_SCANCODE_F6, SDL_SCANCODE_F7, SDL_SCANCODE_F8, SDL_SCANCODE_F9, SDL_SCANCODE_F10,
    SDL_SCANCODE_F11, SDL_SCANCODE_F12,
    SDL_SCANCODE_PRINTSCREEN = 70, SDL_SCANCODE_SCROLLLOCK = 71, SDL_SCANCODE_PAUSE = 72,
    SDL_SCANCODE_INSERT = 73, SDL_SCANCODE_HOME = 74, SDL_SCANCODE_PAGEUP = 75,
    SDL_SCANCODE_DELETE = 76, SDL_SCANCODE_END = 77, SDL_SCANCODE_PAGEDOWN = 78,
    SDL_SCANCODE_RIGHT = 79, SDL_SCANCODE_LEFT = 80, SDL_SCANCODE_DOWN = 81, SDL_SCANCODE_UP = 82,
    SDL_SCANCODE_NUMLOCKCLEAR = 83, SDL_SCANCODE_KP_ENTER = 88,
    SDL_SCANCODE_KP_1 = 89, SDL_SCANCODE_KP_2, SDL_SCANCODE_KP_3, SDL_SCANCODE_KP_4,
    SDL_SCANCODE_KP_5, SDL_SCANCODE_KP_6, SDL_SCANCODE_KP_7, SDL_SCANCODE_KP_8,
    SDL_SCANCODE_KP_9, SDL_SCANCODE_KP_0,
    SDL_SCANCODE_LCTRL = 224, SDL_SCANCODE_LSHIFT = 225, SDL_SCANCODE_LALT = 226,
    SDL_SCANCODE_LGUI = 227, SDL_SCANCODE_RCTRL = 228, SDL_SCANCODE_RSHIFT = 229,
    SDL_SCANCODE_RALT = 230,
    SDL_NUM_SCANCODES = 512
};
typedef int SDL_Keycode;
typedef int SDL_Keymod;
enum { KMOD_NONE = 0, KMOD_LSHIFT = 1, KMOD_RSHIFT = 2, KMOD_LCTRL = 64, KMOD_RCTRL = 128,
       KMOD_LALT = 256, KMOD_RALT = 512, KMOD_NUM = 4096, KMOD_CAPS = 8192 };
#define KMOD_SHIFT (KMOD_LSHIFT | KMOD_RSHIFT)
#define KMOD_CTRL  (KMOD_LCTRL | KMOD_RCTRL)
#define KMOD_ALT   (KMOD_LALT | KMOD_RALT)

// ---- events ----
enum { SDL_QUIT = 256, SDL_KEYDOWN = 768, SDL_KEYUP = 769, SDL_TEXTINPUT = 771,
       SDL_MOUSEMOTION = 1024, SDL_MOUSEBUTTONDOWN = 1025, SDL_MOUSEBUTTONUP = 1026,
       SDL_JOYBUTTONDOWN = 1539, SDL_JOYBUTTONUP = 1540 };
#define SDL_TEXTINPUTEVENT_TEXT_SIZE 32
typedef struct SDL_Keysym { SDL_Scancode scancode; SDL_Keycode sym; Uint16 mod; } SDL_Keysym;
typedef struct SDL_KeyboardEvent { Uint32 type; Uint8 state; Uint8 repeat; SDL_Keysym keysym; } SDL_KeyboardEvent;
typedef struct SDL_TextInputEvent { Uint32 type; char text[SDL_TEXTINPUTEVENT_TEXT_SIZE]; } SDL_TextInputEvent;
typedef struct SDL_JoyButtonEvent { Uint32 type; Uint8 button; Uint8 state; } SDL_JoyButtonEvent;
typedef union SDL_Event {
    Uint32 type;
    SDL_KeyboardEvent key;
    SDL_TextInputEvent text;
    SDL_JoyButtonEvent jbutton;
} SDL_Event;

enum { SDL_IGNORE = 0, SDL_DISABLE = 0, SDL_ENABLE = 1, SDL_QUERY = -1 };
#define SDL_BUTTON(X) (1 << ((X) - 1))
enum { SDL_BUTTON_LEFT = 1, SDL_BUTTON_MIDDLE = 2, SDL_BUTTON_RIGHT = 3 };
enum { SDL_HAT_CENTERED = 0, SDL_HAT_UP = 1, SDL_HAT_RIGHT = 2, SDL_HAT_DOWN = 4, SDL_HAT_LEFT = 8 };

enum { SDL_INIT_TIMER = 1, SDL_INIT_AUDIO = 16, SDL_INIT_VIDEO = 32, SDL_INIT_JOYSTICK = 512 };
enum { SDL_WINDOW_FULLSCREEN = 1, SDL_WINDOW_OPENGL = 2 };
#define SDL_WINDOWPOS_UNDEFINED 0x1FFF0000
enum { SDL_RENDERER_ACCELERATED = 2, SDL_RENDERER_PRESENTVSYNC = 4 };
enum { SDL_BLENDMODE_NONE = 0, SDL_BLENDMODE_BLEND = 1 };
enum { SDL_TEXTUREACCESS_STREAMING = 1 };
enum { SDL_PIXELFORMAT_ARGB8888 = 372645892 };
enum { SDL_MESSAGEBOX_ERROR = 16, SDL_MESSAGEBOX_WARNING = 32, SDL_MESSAGEBOX_INFORMATION = 64 };
#define SDL_HINT_RENDER_SCALE_QUALITY "SDL_RENDER_SCALE_QUALITY"
#define SDL_MUSTLOCK(S) 0

int  SDL_Init(Uint32 flags);
void SDL_Quit(void);
const char *SDL_GetError(void);
int  SDL_SetHint(const char *name, const char *value);

SDL_Window *SDL_CreateWindow(const char *title, int x, int y, int w, int h, Uint32 flags);
void SDL_DestroyWindow(SDL_Window *w);
void SDL_SetWindowGrab(SDL_Window *w, SDL_bool grab);
SDL_Renderer *SDL_CreateRenderer(SDL_Window *w, int index, Uint32 flags);
void SDL_DestroyRenderer(SDL_Renderer *r);
int  SDL_SetRenderDrawBlendMode(SDL_Renderer *r, int mode);
SDL_Texture *SDL_CreateTexture(SDL_Renderer *r, Uint32 format, int access, int w, int h);
void SDL_DestroyTexture(SDL_Texture *t);
int  SDL_ShowCursor(int toggle);
int  SDL_SetRelativeMouseMode(SDL_bool on);
void SDL_WarpMouseInWindow(SDL_Window *w, int x, int y);
int  SDL_ShowSimpleMessageBox(Uint32 flags, const char *title, const char *msg, SDL_Window *w);

SDL_bool SDL_PixelFormatEnumToMasks(Uint32 format, int *bpp, Uint32 *r, Uint32 *g, Uint32 *b, Uint32 *a);
SDL_Surface *SDL_CreateRGBSurface(Uint32 flags, int w, int h, int depth, Uint32 r, Uint32 g, Uint32 b, Uint32 a);
void SDL_FreeSurface(SDL_Surface *s);
int  SDL_SetPaletteColors(SDL_Palette *p, const SDL_Color *colors, int first, int n);
int  SDL_BlitSurface(SDL_Surface *src, const SDL_Rect *sr, SDL_Surface *dst, SDL_Rect *dr);
int  SDL_FillRect(SDL_Surface *dst, const SDL_Rect *r, Uint32 color);
int  SDL_LockSurface(SDL_Surface *s);
void SDL_UnlockSurface(SDL_Surface *s);
Uint32 SDL_MapRGBA(const SDL_PixelFormat *f, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
int  SDL_SaveBMP(SDL_Surface *s, const char *file);

int  SDL_PollEvent(SDL_Event *e);
int  SDL_WaitEvent(SDL_Event *e);
Uint8 SDL_EventState(Uint32 type, int state);
SDL_Keymod SDL_GetModState(void);
Uint32 SDL_GetMouseState(int *x, int *y);
Uint32 SDL_GetRelativeMouseState(int *x, int *y);

int  SDL_NumJoysticks(void);
SDL_Joystick *SDL_JoystickOpen(int index);
void SDL_JoystickClose(SDL_Joystick *j);
void SDL_JoystickUpdate(void);
int  SDL_JoystickNumButtons(SDL_Joystick *j);
int  SDL_JoystickNumHats(SDL_Joystick *j);
Sint16 SDL_JoystickGetAxis(SDL_Joystick *j, int axis);
Uint8 SDL_JoystickGetButton(SDL_Joystick *j, int button);
Uint8 SDL_JoystickGetHat(SDL_Joystick *j, int hat);

Uint32 SDL_GetTicks(void);
void SDL_Delay(Uint32 ms);

// ---- what the port adds: the frame goes out through here ----
// Show W x H 8-bit pixels (PITCH bytes a row) through the 256 colours PAL.
void c4sdl_present(Uint8 *pixels, int w, int h, int pitch, SDL_Color *pal);

#endif
