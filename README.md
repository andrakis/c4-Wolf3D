# src/wolf3d — Wolfenstein 3D on C4IX

id Software's Wolfenstein 3D (1992), as Wolf4SDL has it, compiled by c4lc
for C4IX's libc and played in a desktop window. The shareware episode
ships on the disk as Apogee distributed it, and the game unpacks it the
first time it runs. The whole game works: the display, the keyboard,
music and sound effects, and the game itself. It is played from the
keyboard; the mouse is not wired up yet.
[docs/c4ix-wolf3d.md](../../docs/c4ix-wolf3d.md) is the whole story: how
it was ported, what c4lc needed, and how fast it goes.

## Where it comes from

- **The game:** [Wolf4SDL](https://github.com/KS-Presto/Wolf4SDL) v2.0 by
  Moritz "Ripper" Kroll and the community, KS-Presto's fork, which went
  back to C. That is commit `dc8b250` ("Farewell GitHub", 2024; the fork
  carries on at bitbucket.org/ks-presto/wolf4sdl). It is used
  under the GNU GPL v2 (`license-gpl.txt`); id's own licence for the
  original source is `license-id.txt`. The MAME OPL emulator, under the
  MAME licence, is left out: the AdLib is libjs's sound card, whose OPL
  is DOSBox's dbopl ported to JavaScript (`libjs/opl.js`).
- **The data:** `data/1wolf14.zip` is Apogee's shareware v1.4 package,
  complete and unmodified, as its `VENDOR.DOC` asks of anyone who passes
  it on. It is 856,401 bytes, SHA-256
  `cb2a2ef7ecef14152c65ff93cc3b84fbd3e8eb0c5c1de41a6fc8cdef559451a8`. It
  came from RPM Fusion's `wolf3d-shareware` source package; its upstream
  is `ftp://ftp.3drealms.com/share/1wolf14.zip`.

## What is here

| file | what |
|---|---|
| `id_*.c`, `wl_*.c`, `*.h`, `signon.c`, `wolfpal.inc` | Wolf4SDL, with the changes below |
| `sdl/SDL.h`, `sdl/SDL_mixer.h` | the part of SDL 2 the game uses, and no more |
| `c4sdl.c` | those functions, over a platform (`plat.h`): a window, key events, the clock and a sound card; SDL_mixer's calls over that card; the timedemo's virtual clock and frame checksum |
| `plat-c4ix.c` | the platform on C4IX: the Terminal window as a canvas (`window.h`'s `win_fb`), and libjs's sound card (`libjs/guest/snd.h`) |
| `plat-host.c` | the platform on a host, built with gcc: no window and no sound, frames to `.ppm` on request |
| `c4file.c`, `c4file.h` | files read and written whole, since C4IX had no seek when they were written (`lseek` came with M7c, docs/busybox-port.md) |
| `wolfinst.c`, `explode.c`, `arc.c` | the first run: `1wolf14.zip` (inflate, from libarc) and the installer's `W3DSW14.SHR` (PKWARE DCL "implode", decompressed by `explode.c`) |
| `c4inc/stdint.h` | the fixed-width types, which C4IX's libc lacks |
| `tools/unshr.c` | the same unpacking on a host, for the test |

## The changes to Wolf4SDL

Each change is marked `c4sdl:` in the source. Everything else is
Wolf4SDL's own code. The port used to write around what c4lc lacked: 2-D
arrays, structs laid over the files' bytes under `#pragma pack(1)`, a struct passed by value, a
typedef inside a function, float and `sizeof` initializers of globals,
array parameters, braces around a scalar initializer, and several
declarators in one declaration. c4lc has all of these now
([docs/c4lc-design.md](../../docs/c4lc-design.md) 14.8), so those
rewrites are gone.

- **A bug of Wolf4SDL's own:** `itoa`/`ltoa` sized their output by `strlen()` of the caller's uninitialized buffer, so the status bar's digits depended on what the stack held.
- **Speed:** `FixedMul` in 32-bit pieces, `int`s rather than `int16_t` in the scalers, and the wall column drawn through a pointer (docs/c4ix-wolf3d.md, "How fast").
- **Sound through libjs's sound card.** `id_sd.c`'s AdLib calls (`YM3812Write` and the rest) go to the card's OPL, and the game's own music player runs from its loop, keeping the card 100 ms ahead. The digitized sounds play as the game's 8-bit samples at 7,042 Hz, not converted. The PC speaker's sounds are silent. With no card (`plat-host.c`) the game runs silent, as before.
- **The original 320x200 in 8 bits**, presented through a palette by the host (`GUI_BLIT8`).
- **`--timedemo [N]`** plays the first N demos in virtual time and prints each one's frame count and a checksum of every frame shown. The gcc and c4lc builds must print the same.

## Building and testing

- `make wolf3d-objs` compiles each unit with c4lc into `.wolf3d/`. `build-images.sh` links them against C4IX's libc as `/usr/games/wolf3d/wolf3d.c4r`, beside the zip.
- `make wolf3d-host` is the gcc build.
- `make test-wolf3d` plays demo 0 both ways and compares the frames. A timedemo is silent unless `--sound` is given; `node libjs/cli.js --wav FILE` records what it plays.
