# Wolfenstein 3D's data

`1wolf14.zip` is Apogee's shareware package of Wolfenstein 3D v1.4,
856,401 bytes, unmodified, as 3D Realms distributed it from
`ftp.3drealms.com/share/`. That server is gone; the Internet Archive
kept a copy. It holds the first episode.

Its `VENDOR.DOC` allows the package to be passed on unchanged, with every
file in it, and marked as shareware. The game unpacks it itself the first
time it runs on C4IX (`wolfinst.c`); on the bare machine and under C4DOS
c4's `build-images.sh` unpacks it with `tools/unshr.c`.

## Getting it

`1wolf14.zip` is not in this repository. In c4, `make wolf3d-data` puts it here
with c4's `games/fetch.sh`:
- from `$WOLF3D_ZIP`, when that names your copy;
- or else from the first URL in `MIRRORS` that gives exactly this file.

Either way it is refused unless its SHA-256 is
`cb2a2ef7ecef14152c65ff93cc3b84fbd3e8eb0c5c1de41a6fc8cdef559451a8`.
Every URL in `MIRRORS` was checked to give exactly that file
(2026-10-25).
