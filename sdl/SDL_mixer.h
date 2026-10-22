// SDL_mixer.h -- the mixer calls id_sd.c makes, over the platform's sound
// card (plat.h): c4sdl.c. With no card Mix_OpenAudioDevice fails, and
// id_sd.c runs silent. A chunk is the game's own 8-bit samples, at their
// own rate (c4sdl_rawchunk), not a WAV converted to the device's.

#ifndef C4SDL_MIXER_H
#define C4SDL_MIXER_H 1

#include "SDL.h"

#define MIX_CHANNELS 8
#define AUDIO_S16 32784
#define SDL_AUDIO_ALLOW_FREQUENCY_CHANGE 1

typedef struct Mix_Chunk { int allocated; Uint8 *abuf; Uint32 alen; Uint8 volume; int rate; } Mix_Chunk;
typedef struct SDL_RWops { Uint8 *base; int size; } SDL_RWops;

int  Mix_OpenAudioDevice(int frequency, Uint16 format, int channels, int chunksize, const char *device, int changes);
int  Mix_QuerySpec(int *frequency, Uint16 *format, int *channels);
const char *Mix_GetError(void);
int  Mix_ReserveChannels(int num);
int  Mix_GroupChannels(int from, int to, int tag);
int  Mix_GroupAvailable(int tag);
int  Mix_GroupOldest(int tag);
int  Mix_HaltChannel(int channel);
int  Mix_SetPanning(int channel, Uint8 left, Uint8 right);
int  Mix_PlayChannel(int channel, Mix_Chunk *chunk, int loops);
void Mix_FreeChunk(Mix_Chunk *chunk);
Mix_Chunk *Mix_LoadWAV_RW(SDL_RWops *src, int freesrc);
void Mix_HookMusic(void (*fn)(void *, Uint8 *, int), void *arg);
void Mix_ChannelFinished(void (*fn)(int));
int  Mix_SetPostMix(void (*fn)(void *, Uint8 *, int), void *arg);
SDL_RWops *SDL_RWFromMem(void *mem, int size);
void Mix_CloseAudio(void);
// c4sdl: unsigned 8-bit mono samples at rate, kept where they are
Mix_Chunk *c4sdl_rawchunk(Uint8 *samples, int len, int rate);
// c4sdl: keep the music ahead of the sound card (c4sdl.c calls it itself
// from the event pump, SDL_Delay and each frame)
void c4sdl_sound_service(void);

#endif
