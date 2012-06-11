#ifndef _sdlinc_h_
#define _sdlinc_h_

#if (SDLMAME_SDL2)
#include <SDL2/SDL.h>
#include <SDL2/SDL_version.h>
// on win32 this includes windows.h by itself and breaks us!
#ifndef SDLMAME_WIN32
#include <SDL2/SDL_syswm.h>
#endif
#else
#include <SDL/SDL.h>
#include <SDL/SDL_version.h>
// on win32 this includes windows.h by itself and breaks us!
#ifndef SDLMAME_WIN32
#include <SDL/SDL_syswm.h>
#endif
#endif

#endif
