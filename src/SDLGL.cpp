#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "SDLGL.h"
SDLResVidModes sdlResVideoModes[18] = {{320,240},{320,480},{480,320},{640,480},{800,600},{1024,768},{320,240},{320,240},{320,240},{320,240},{320,240},{320,240},{320,240},{320,240},{320,240},{320,240},{320,240},{320,240}};
SDLGL::SDLGL() : initialized(false), window(NULL), resolutionIndex(0), oldResolutionIndex(0), winVidWidth(320), winVidHeight(240), vidWidth(320), vidHeight(240), windowMode(0), oldWindowMode(0), vSync(true), oldVSync(true) {}
SDLGL::~SDLGL() { if (initialized) { retrofw_ShutdownGL(); SDL_Quit(); initialized = false; } }
bool SDLGL::Initialize() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) < 0) return false;
    SDL_ShowCursor(SDL_DISABLE);
    window = SDL_SetVideoMode(320, 240, 16, SDL_HWSURFACE);
    if (!window) return false;
    retrofw_InitGL(window);
    initialized = true;
    return true;
}
void SDLGL::Error(const char* fmt, ...) {
    char buf[1024]; va_list a; va_start(a, fmt); vsnprintf(buf, sizeof(buf), fmt, a); va_end(a);
    fprintf(stderr, "ERROR: %s\n", buf); exit(1);
}
void SDLGL::transformCoord2f(float* x, float* y) { (void)x; (void)y; }
void SDLGL::centerMouse(int x, int y) { (void)x; (void)y; }
void SDLGL::updateWinVid(int w, int h) { (void)w; (void)h; }
void SDLGL::updateVideo() {}
void SDLGL::restore() {}
