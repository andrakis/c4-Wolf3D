# games/wolf3d — Wolfenstein 3D on C4IX

id Software's Wolfenstein 3D (1992), as Wolf4SDL has it, compiled by c4lc
for C4IX's libc and played in a desktop window. The shareware episode
ships on the disk as Apogee distributed it, and the game unpacks it the
first time it runs. The whole game works: the display, the keyboard,
music and sound effects, and the game itself. It is played from the
keyboard; the mouse is not wired up yet.
[docs/c4ix-wolf3d.md](../../docs/c4ix-wolf3d.md) is the whole story: how
it was ported, what c4lc needed, and how fast it goes.

## Where it runs

The same game objects run in four places. Only the platform underneath
changes:

| where | how | platform |
|---|---|---|
| C4IX, the whole display | `wolf3d`, or Go > Wolfenstein 3D, the desktop giving way until it ends (`include/display.h`); the page's `?system=wolf3d-c4ix` | `plat-c4ix.c` handing over to `plat-gui.c` |
| a C4IX desktop window | `wolf3d --window` | `plat-c4ix.c`: the Terminal window as a canvas |
| C4DOS | `WOLF3D` at the `A>` prompt; the page's `?system=c4dos` | `plat-bare.c`, over `plat-gui.c` |
| the bare machine | `wolf3d.c4r` as the machine's image; the page's `?system=wolf3d` | `plat-bare.c`, over `plat-gui.c` |

`plat-gui.c` draws on libjs's display itself (`libjs/guest/gui.h`): the
game's 320x200 scaled to 640x480 by the host, with keys from the display. The last
two have no C4IX: the game is linked whole with libc and with `bare.c`,
libc's system layer over the machine's own opcodes
([src/c4ix/libc/README.md](../c4ix/libc/README.md), "Without C4IX"). They
read the game's files already unpacked, from build-images.sh's
`images/games` disk, since neither can write a drive. All four play the
same timedemo frames as the gcc build.

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
| `plat-c4ix.c` | the platform on C4IX: the whole display (`plat-gui.c`), taken from the desktop and given back, or with `--window` the Terminal window as a canvas (`window.h`'s `win_fb`) |
| `plat-gui.c` | the display drawn directly (`libjs/guest/gui.h`), the browser's key codes as SDL scancodes, and libjs's sound card (`libjs/guest/snd.h`) for every platform that has one |
| `plat-bare.c` | the platform with no C4IX: the bare machine and C4DOS, over `plat-gui.c` |
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
- **`--secrets` and `--nodeath`**, two switches for fun (below).
- **The registered game**, `wolf3d6`, built again with `-D WOLF_FULL` (below).

## For fun: `--secrets` and `--nodeath`

Neither changes the game unless it is given, and the demos play as
before with either (with `--secrets` a timedemo's checksums change, as
the frames do).

- **`--secrets`** shows where the secrets are. A pushwall that has not
  been pushed yet (plane 1's `PUSHABLETILE`) is drawn a little gold: the
  same texture, each colour remapped to the palette's nearest to a
  brighter, warmer one (`SecretPost` in `wl_draw.c`). Once pushed it
  looks like any other wall.
- **`--nodeath`**: instead of dying, the player goes down to 0 health
  with the death cry and limps on: a lurching step at half speed, then a
  drag at an eighth, and turning at half speed. The status bar's face is
  the bloodiest one, passing out now and then. Enemies can still shoot,
  but it cannot get worse. Health is picked up as usual, so it still
  takes 100 to be whole again, and the limp ends with the first point
  back (`LIMPING` in `wl_agent.c`). The demos never limp, so that they
  play as they were recorded.

To play with them:

| where | how |
|---|---|
| the page, the bare machine | `?system=wolf3d&args=--secrets+--nodeath`, or Configuration > Arguments |
| the page, C4DOS | `?system=c4dos`, then `WOLF3D --secrets --nodeath` at the `A>` prompt |
| the C4IX desktop | Go > Run... `wolf3d --secrets --nodeath`, or the same in a Terminal |

`--tedlevel 3` starts straight in E1M4, whose first room has a pushwall
in its south-west corner; add `--goobers` for id's debug keys
(Backspace+Left Shift+Alt in the game, then Tab+H hurts the player by 16).

## The registered game: `wolf3d6`

Wolf4SDL is built for one set of data files, since their graphics are
numbered differently: `wolf3d` is the shareware episode (`UPLOAD`,
`gfxv_apo.h`). `wolf3d6` is the same game built again with `-D WOLF_FULL`,
which `version.h` turns into `GOODTIMES`: id's v1.4 of the six episodes,
as GT, Activision, GOG and Steam sell it, whose `*.WL6` files have 149
graphics chunks (`gfxv_wl6.h`, from Wolf4SDL; `VGAHEAD.WL6` is 450 bytes).
Apogee's own registered v1.4 (162 chunks, with Read This!) is not it.

Those files are the person's own, so no disk has them. `wolf3d6` finds
them where it is run, or on the host's drive 0, where the page's
**Uploads** put them (libjs/README.md): drop the eight `*.WL6` files on
the page, or add them with the Uploads button, then Boot with them.
Without them it says so and stops. On C4IX the uploads are not in its
own tree but opened from the host when nothing there has the name
(`sys_open`), so its check for `vswap.wl6` opens the file rather than
`stat()`ing it (`wl_menu.c`), and nothing is unpacked (`wolfinst.c`).
Its configuration and saved games are written where it runs, as
`wolf3d`'s are.

| where | how |
|---|---|
| the page, the bare machine | `?system=wolf3d6`, or Wolfenstein 3D, registered |
| the page, C4DOS | `?system=c4dos`, then `WOLF3D6` at the `A>` prompt |
| C4IX | `wolf3d6`, in a Terminal or from Go > Run... |

`--secrets` and `--nodeath` work there too.

## Building and testing

- `make wolf3d-objs` compiles each unit with c4lc into `.wolf3d/`. `build-images.sh` links them against C4IX's libc as `/usr/games/wolf3d/wolf3d.c4r`, beside the zip.
- `make wolf3d-bare-objs` compiles `plat-bare.c`, and libc without `-fpic` with `bare.c` for its system layer, into `.wolf3d-bare/`. `build-images.sh` links them with the same game objects as `images/games/wolf3d.c4r`, on a disk with the game unpacked, Doom (games/doom) and C4DOS's files.
- `make wolf3d6-objs` compiles the units that see `version.h` again with `-D WOLF_FULL` into `.wolf3d6/`, and `build-images.sh` links them with the rest of `.wolf3d/`'s as `/usr/games/wolf3d/wolf3d6.c4r` and `images/games/wolf3d6.c4r`. `make c4bb-images` builds them with the others.
- `make wolf3d-host` is the gcc build, and `make wolf3d6-host` the registered one.
- `make test-wolf3d` plays demo 0 built by gcc, inside C4IX, on the bare machine and under C4DOS, and compares the frames. A timedemo is silent unless `--sound` is given; `node libjs/cli.js --wav FILE` records what it plays.
- To compare how fast the game runs between machines, add `--show`: the frames go to the display (or to the window, with `--window`) as well, so the time counts the drawing. Each demo prints a speed line before its checksum, `speed: demo 0: 1234 frames in 9.876 s, 124.9 fps`, by the real clock. In the page that clock is the wall's; under `cli.js`, unpaced, it is cycles over the MHz asked for.
