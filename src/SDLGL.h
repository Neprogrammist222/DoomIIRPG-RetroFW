#ifndef __SDLGL_H__
#define __SDLGL_H__
#include "gl_shim.h"
typedef struct SDLResVidModes_s { int width, height; } SDLResVidModes;
extern SDLResVidModes sdlResVideoModes[18];
class SDLGL {
private:
    bool initialized;
public:
    SDL_Surface* window;
    int resolutionIndex, oldResolutionIndex;
    int winVidWidth, winVidHeight, vidWidth, vidHeight;
    int windowMode, oldWindowMode;
    bool vSync, oldVSync;
    SDLGL(); ~SDLGL();
    bool Initialize();
    void Error(const char* fmt, ...);
    void transformCoord2f(float* x, float* y);
    void centerMouse(int x, int y);
    void updateWinVid(int w, int h);
    void updateVideo();
    void restore();
};
#endif
