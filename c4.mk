# c4.mk -- this game's part of c4's Makefile. c4 includes it from
# games/wolf3d/c4.mk, its submodule, and runs it from c4's root, with c4's
# tools and variables (games/README.md in c4). Without it, c4 builds
# everything else, and test-wolf3d says it was skipped.

# WOLFENSTEIN 3D (games/wolf3d, docs/c4ix-wolf3d.md): Wolf4SDL in c4lc's C,
# for C4IX's libc. A unit to an object, made here so that only what
# changed is compiled again -- the whole game is minutes of c4sp32, and
# -j helps -- and linked by build-images.sh against the libc it builds.
WOLF       := games/wolf3d
WOLF_UNITS := arc explode c4sdl c4file plat-c4ix plat-gui wolfinst id_ca id_in id_pm id_sd id_us \
              id_vh id_vl signon wl_act1 wl_act2 wl_agent wl_debug wl_draw wl_game \
              wl_inter wl_main wl_menu wl_play wl_scale wl_state wl_text wl_utils
WOLF_HDRS  := $(wildcard $(WOLF)/*.h $(WOLF)/*.inc $(WOLF)/sdl/*.h $(WOLF)/c4inc/*.h) \
              $(wildcard src/c4ix/libc/include/*.h src/c4ix/libc/include/sys/*.h) \
              src/c4ix/include/window.h src/c4ix/include/archive.h \
              libjs/guest/snd.h libjs/guest/snd.c
WOLF_CC    := ./c4sp32 -c 400000 src/c4sp/lisp/c4lc.lisp -O $(IMG_G) -mfuse -fno-builtin -c -D C4IX \
              -I src/c4ix/libc/include -I src/c4ix/include -I src/c4ix/user -I libjs/guest \
              -I $(WOLF)/c4inc -I $(WOLF)/sdl -I $(WOLF)
wolf3d-objs: $(WOLF_UNITS:%=.wolf3d/%.c4o)
.wolf3d/%.c4o: $(WOLF)/%.c $(WOLF_HDRS) $(C4LC_LISP) .img_g | c4sp32
	@mkdir -p .wolf3d
	$(WOLF_CC) $< $@ > /dev/null
# arc.c is libarc's archive.c, built against libc
.wolf3d/arc.c4o: src/c4ix/lib/archive.c
PHONY += wolf3d-objs

# ... and for the bare machine and C4DOS (README.md, "Where it runs"):
# the same game objects, with plat-bare.c for plat-c4ix.c, linked by
# c4-images.sh with c4's bare libc (libc-bare-objs).
wolf3d-bare-objs: .wolf3d-bare/plat-bare.c4o libc-bare-objs
.wolf3d-bare/plat-bare.c4o: $(WOLF)/plat-bare.c $(WOLF_HDRS) $(C4LC_LISP) .img_g | c4sp32
	@mkdir -p .wolf3d-bare
	$(WOLF_CC) $< $@ > /dev/null
PHONY += wolf3d-bare-objs

# ... and the registered game, wolf3d6 (README.md, "The registered
# game"): the units that see version.h, again with -D WOLF_FULL, into
# .wolf3d6/. c4-images.sh links them with the rest of .wolf3d/'s.
WOLF6_UNITS := wolfinst id_ca id_in id_pm id_sd id_us id_vh id_vl signon wl_act1 wl_act2 wl_agent \
               wl_debug wl_draw wl_game wl_inter wl_main wl_menu wl_play wl_scale wl_state wl_text
wolf3d6-objs: $(WOLF6_UNITS:%=.wolf3d6/%.c4o)
.wolf3d6/%.c4o: $(WOLF)/%.c $(WOLF_HDRS) $(C4LC_LISP) .img_g | c4sp32
	@mkdir -p .wolf3d6
	$(WOLF_CC) -D WOLF_FULL $< $@ > /dev/null
PHONY += wolf3d6-objs

# The data, which is not in the repository: data/1wolf14.zip, the shareware
# package as it was distributed, checked by its SHA-256. c4's
# games/fetch.sh takes it from $WOLF3D_ZIP when that names a copy, or
# else from the mirrors in data/MIRRORS (data/README.md).
wolf3d-data: $(WOLF)/data/1wolf14.zip
$(WOLF)/data/1wolf14.zip:
	@sh games/fetch.sh $@ cb2a2ef7ecef14152c65ff93cc3b84fbd3e8eb0c5c1de41a6fc8cdef559451a8 WOLF3D_ZIP $$(cat $(WOLF)/data/MIRRORS 2>/dev/null)
PHONY += wolf3d-data

# The same game built by gcc for the host, with no window: the reference
# the C4IX build is held to (test-wolf3d).
WOLF_HOST_SRCS := $(filter-out $(WOLF)/plat-c4ix.c $(WOLF)/plat-gui.c $(WOLF)/wolfinst.c $(WOLF)/explode.c $(WOLF)/arc.c, \
                  $(WOLF_UNITS:%=$(WOLF)/%.c)) $(WOLF)/plat-host.c
wolf3d-host: $(WOLF_HOST_SRCS) $(wildcard $(WOLF)/*.h $(WOLF)/sdl/*.h)
	gcc -O2 -std=gnu99 -w -Werror-implicit-function-declaration -I$(WOLF)/sdl -I$(WOLF) -o $@ $(WOLF_HOST_SRCS) -lm
wolf3d6-host: $(WOLF_HOST_SRCS) $(wildcard $(WOLF)/*.h $(WOLF)/sdl/*.h)
	gcc -O2 -std=gnu99 -w -Werror-implicit-function-declaration -D WOLF_FULL -I$(WOLF)/sdl -I$(WOLF) -o $@ $(WOLF_HOST_SRCS) -lm

# test-wolf3d: demo 0 in virtual time, built both ways, must show the same
# 1214 frames. Inside C4IX the game first unpacks 1wolf14.zip from the
# disk, so that is tested too; on the host tools/unshr.c does it. Then the
# same demo on the bare machine and under C4DOS (plat-bare.c, libc's
# bare.c), from build-images.sh's wolf3d disk -- the two together, as
# they share nothing -- and C4DOS must be back at its prompt after the
# game exits, to run VER. About six minutes, nearly all of it the runs
# on c4m.
test-wolf3d: wolf3d-data wolf3d-host c4bb-images
	@rm -rf .wolf3d_t && mkdir -p .wolf3d_t
	@python3 -I -c "import zipfile, sys; zipfile.ZipFile(sys.argv[1]).extract('W3DSW14.SHR', sys.argv[2])" $(WOLF)/data/1wolf14.zip .wolf3d_t
	@gcc -O2 -o .wolf3d_t/unshr $(WOLF)/tools/unshr.c
	@cd .wolf3d_t && ./unshr W3DSW14.SHR . > /dev/null
	@cd .wolf3d_t && ../wolf3d-host --timedemo 1 2>/dev/null | grep '^timedemo' | sed 's/, [0-9]* ms$$//' > host.txt
	@printf 'wolf3d --timedemo 1\nexit\n' | timeout 900 node libjs/cli.js -m 64 -d src/c4bb/images/disk src/c4bb/images/c4ix32.c4r 2>/dev/null \
	   | grep '^timedemo' | sed 's/, [0-9]* ms$$//' > .wolf3d_t/c4ix.txt
	@cat .wolf3d_t/host.txt
	@test -s .wolf3d_t/host.txt && cmp -s .wolf3d_t/host.txt .wolf3d_t/c4ix.txt \
	   || { echo "test-wolf3d: C4IX's frames differ:"; cat .wolf3d_t/c4ix.txt; exit 1; }
	@cd src/c4bb/images/games && { timeout 900 node ../../../../libjs/cli.js -m 64 -d . wolf3d.c4r --timedemo 1 \
	     > ../../../../.wolf3d_t/bare.out 2>/dev/null & \
	   printf 'wolf3d --timedemo 1\nver\n' | timeout 900 node ../../../../libjs/cli.js -m 64 -d . ../c4dos32.c4r \
	     > ../../../../.wolf3d_t/dos.out 2>/dev/null; wait; }
	@grep '^timedemo' .wolf3d_t/bare.out | sed 's/, [0-9]* ms$$//' | cmp -s .wolf3d_t/host.txt - \
	   || { echo "test-wolf3d: the bare machine's frames differ:"; tail -5 .wolf3d_t/bare.out; exit 1; }
	@grep '^timedemo' .wolf3d_t/dos.out | sed 's/, [0-9]* ms$$//' | cmp -s .wolf3d_t/host.txt - \
	   || { echo "test-wolf3d: C4DOS's frames differ:"; tail -5 .wolf3d_t/dos.out; exit 1; }
	@grep -A1 '^timedemo' .wolf3d_t/dos.out | grep -Eq '^A(:\\)?>C4DOS version' \
	   || { echo "test-wolf3d: C4DOS did not come back to its prompt:"; tail -5 .wolf3d_t/dos.out; exit 1; }
	@rm -rf .wolf3d_t
	@echo "test-wolf3d: OK"
PHONY += test-wolf3d

GAME_OBJS  += wolf3d-objs wolf3d-bare-objs wolf3d6-objs
GAME_TESTS += test-wolf3d
