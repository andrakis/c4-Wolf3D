// WL_UTILS.C

#include "wl_utils.h"


/*
===================
=
= safe_malloc
=
= Wrapper for malloc with a NULL check
=
===================
*/

void *safe_malloc (size_t size, const char *fname, uint32_t line)
{
    void *ptr;

    ptr = malloc(size);

    if (!ptr)
        Quit ("SafeMalloc: Out of memory at %s: line %u",fname,line);

    return ptr;
}


// c4sdl: in 32-bit pieces. ((int64_t)a * b + 0x8000) >> 16, to its low 32
// bits, is the sum below: a = ah * 2^16 + al, with ah = a >> 16 (arithmetic)
// and al = a & 0xffff, and likewise b. c4lc does int64_t at 32 bits in
// helper calls, and this is the hottest multiply in the game.
fixed FixedMul (fixed a, fixed b)
{
    uint32_t ah = (uint32_t)(a >> 16), al = (uint32_t)a & 0xffff;
    uint32_t bh = (uint32_t)(b >> 16), bl = (uint32_t)b & 0xffff;
    return (fixed)(((ah * bh) << 16) + ah * bl + al * bh + ((al * bl + 0x8000) >> 16));
}

fixed FixedDiv (fixed a, fixed b)
{
	int64_t c = ((int64_t)a << FRACBITS) / (int64_t)b;

	return (fixed)c;
}

uint16_t ReadShort (void *ptr)
{
    unsigned value;
    byte     *work;

    work = ptr;
    value = work[0] | (work[1] << 8);

    return value;
}

uint32_t ReadLong (void *ptr)
{
    uint32_t value;
    byte     *work;

    work = ptr;
    value = work[0] | (work[1] << 8) | (work[2] << 16) | (work[3] << 24);

    return value;
}


void Error (const char *string)
{
    SDL_ShowSimpleMessageBox (SDL_MESSAGEBOX_ERROR,"Wolf4SDL",string,NULL);
}

void Help (const char *string)
{
    SDL_ShowSimpleMessageBox (SDL_MESSAGEBOX_INFORMATION,"Wolf4SDL",string,NULL);
}
