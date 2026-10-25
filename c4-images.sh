# c4-images.sh c4ix|bare -- Wolfenstein 3D's part of c4's build-images.sh,
# which sources it from c4's root (games/README.md in c4)
GAMEDIR=games/wolf3d
# what wolf3d6 takes from .wolf3d/: the units that do not see version.h
wolf6_shared() { for u in arc explode c4sdl c4file plat-gui wl_utils; do echo .wolf3d/$u.c4o; done; }
case "$1" in
c4ix)
    # linked against the libc kept from the last time C4IX was built, and
    # put on the disk beside Apogee's shareware package, which the game
    # unpacks the first time it runs
    if ls .wolf3d/*.c4o > /dev/null 2>&1 && [ -f $PB/lib/libc.c4s ]; then
        say "linking Wolfenstein 3D for C4IX"
        ./c4rlink32 .wolf3d/*.c4o -l $PB/lib/libc.c4s -o $DISK/c4ix-wolf3d.c4r > /dev/null
        if [ -f $GAMEDIR/data/1wolf14.zip ]; then cp $GAMEDIR/data/1wolf14.zip $DISK/1wolf14.zip; fi
    fi
    # the registered game, wolf3d6: .wolf3d6/'s units for .wolf3d/'s own,
    # with no data (the person's own, from the page's Uploads)
    if ls .wolf3d6/*.c4o > /dev/null 2>&1 && [ -f $PB/lib/libc.c4s ]; then
        ./c4rlink32 .wolf3d6/*.c4o $(wolf6_shared) .wolf3d/plat-c4ix.c4o -l $PB/lib/libc.c4s             -o $DISK/c4ix-wolf3d6.c4r > /dev/null
    fi ;;
bare)
    # with plat-bare.c and c4's bare libc; its files unpacked beside the
    # package they come from, unmodified, as its VENDOR.DOC asks
    if ls .wolf3d/*.c4o > /dev/null 2>&1 && [ -f .wolf3d-bare/plat-bare.c4o ]; then
        item wolf3d
        ./c4rlink32 $(ls .wolf3d/*.c4o | grep -v '/plat-c4ix.c4o$') .wolf3d-bare/plat-bare.c4o .libc-bare/*.c4o \
            -o $GAMESDISK/wolf3d.c4r > /dev/null
        if [ -f $GAMEDIR/data/1wolf14.zip ]; then
            cp $GAMEDIR/data/1wolf14.zip $GAMESDISK/
            python3 -I -c "import zipfile, sys; zipfile.ZipFile(sys.argv[1]).extract('W3DSW14.SHR', sys.argv[2])" \
                $GAMEDIR/data/1wolf14.zip $GAMESDISK
            gcc -O2 -o .wolf3d-bare/unshr $GAMEDIR/tools/unshr.c
            .wolf3d-bare/unshr $GAMESDISK/W3DSW14.SHR $GAMESDISK > /dev/null
            rm -f $GAMESDISK/W3DSW14.SHR $GAMESDISK/*.exe
        fi
        GAMES_ON_DISK="$GAMES_ON_DISK WOLF3D"
    fi
    if ls .wolf3d6/*.c4o > /dev/null 2>&1 && [ -f .wolf3d-bare/plat-bare.c4o ]; then
        item wolf3d6
        ./c4rlink32 .wolf3d6/*.c4o $(wolf6_shared) .wolf3d-bare/plat-bare.c4o .libc-bare/*.c4o             -o $GAMESDISK/wolf3d6.c4r > /dev/null
    fi ;;
esac
