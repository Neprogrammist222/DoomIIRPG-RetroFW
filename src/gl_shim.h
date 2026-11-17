typedef float GLclampf;
#ifndef GL_SHIM_H
#define GL_SHIM_H

#include <SDL/SDL.h>
#include <GL/gl.h>
#include <zbuffer.h>
#include <stdint.h>


typedef void SDL_GameController;
typedef int SDL_JoystickID;
typedef void SDL_Haptic;
typedef int SDL_Scancode;

#define SDL_SCANCODE_UNKNOWN SDLK_UNKNOWN
#define SDL_SCANCODE_UP SDLK_UP
#define SDL_SCANCODE_DOWN SDLK_DOWN
#define SDL_SCANCODE_LEFT SDLK_LEFT
#define SDL_SCANCODE_RIGHT SDLK_RIGHT
#define SDL_SCANCODE_W SDLK_UP
#define SDL_SCANCODE_S SDLK_DOWN
#define SDL_SCANCODE_A SDLK_TAB
#define SDL_SCANCODE_D SDLK_BACKSPACE
#define SDL_SCANCODE_Z SDLK_LCTRL
#define SDL_SCANCODE_X SDLK_LALT
#define SDL_SCANCODE_C SDLK_SPACE
#define SDL_SCANCODE_RETURN SDLK_RETURN
#define SDL_SCANCODE_ESCAPE SDLK_ESCAPE
#define SDL_SCANCODE_TAB SDLK_LSHIFT
#define SDL_SCANCODE_I SDLK_LSHIFT
#define SDL_SCANCODE_O SDLK_SPACE
#define SDL_SCANCODE_P SDLK_RETURN
#define SDL_SCANCODE_B SDLK_ESCAPE
#define SDL_SCANCODE_SPACE SDLK_SPACE
#define SDL_SCANCODE_LCTRL SDLK_LCTRL
#define SDL_SCANCODE_LALT SDLK_LALT
#define SDL_SCANCODE_LSHIFT SDLK_LSHIFT
#define SDL_SCANCODE_BACKSPACE SDLK_BACKSPACE

enum {
    SDL_CONTROLLER_BUTTON_A = 0, SDL_CONTROLLER_BUTTON_B, SDL_CONTROLLER_BUTTON_X, SDL_CONTROLLER_BUTTON_Y,
    SDL_CONTROLLER_BUTTON_BACK, SDL_CONTROLLER_BUTTON_GUIDE, SDL_CONTROLLER_BUTTON_START,
    SDL_CONTROLLER_BUTTON_LEFTSTICK, SDL_CONTROLLER_BUTTON_RIGHTSTICK,
    SDL_CONTROLLER_BUTTON_LEFTSHOULDER, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
    SDL_CONTROLLER_BUTTON_DPAD_UP, SDL_CONTROLLER_BUTTON_DPAD_DOWN,
    SDL_CONTROLLER_BUTTON_DPAD_LEFT, SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
    SDL_CONTROLLER_BUTTON_MAX,
    SDL_CONTROLLER_AXIS_LEFTX = 0, SDL_CONTROLLER_AXIS_LEFTY,
    SDL_CONTROLLER_AXIS_RIGHTX, SDL_CONTROLLER_AXIS_RIGHTY,
    SDL_CONTROLLER_AXIS_TRIGGERLEFT, SDL_CONTROLLER_AXIS_TRIGGERRIGHT,
    SDL_CONTROLLER_AXIS_MAX,
    SDL_CONTROLLERDEVICEADDED = 0x653,
    SDL_CONTROLLERDEVICEREMOVED = 0x654,
    SDL_CONTROLLERDEVICEREMAPPED = 0x655,
    SDL_CONTROLLERBUTTONDOWN = 0x651,
    SDL_CONTROLLERBUTTONUP = 0x652,
    SDL_CONTROLLERAXISMOTION = 0x650,
    SDL_WINDOWEVENT = 0x200,
    SDL_WINDOWEVENT_CLOSE = 14,
    SDL_WINDOWEVENT_SIZE_CHANGED = 6,
    SDL_MOUSEWHEEL = 0x403
};

#ifdef __cplusplus
extern "C" {

#ifndef GL_TEXTURE0
#define GL_TEXTURE0 0x84C0
#endif
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif
#ifndef GL_GENERATE_MIPMAP
#define GL_GENERATE_MIPMAP 0x8191
#endif
#ifndef GL_FOG
#define GL_FOG 0x0B60
#define GL_FOG_MODE 0x0B65
#define GL_FOG_DENSITY 0x0B62
#define GL_FOG_START 0x0B63
#define GL_FOG_END 0x0B64
#define GL_FOG_COLOR 0x0B66
#define GL_EXP 0x0800
#define GL_EXP2 0x0801
#endif
#ifndef GL_ALPHA_TEST
#define GL_ALPHA_TEST 0x0BC0
#endif
#ifndef GL_SCISSOR_TEST
#define GL_SCISSOR_TEST 0x0C11
#endif
#ifndef GL_STENCIL_TEST
#define GL_STENCIL_TEST 0x0B90
#endif
#ifndef GL_COMBINE
#define GL_COMBINE 0x8570
#define GL_COMBINE_RGB 0x8571
#define GL_COMBINE_ALPHA 0x8572
#define GL_SOURCE0_RGB 0x8580
#define GL_SOURCE1_RGB 0x8581
#define GL_SOURCE0_ALPHA 0x8588
#define GL_SOURCE1_ALPHA 0x8589
#define GL_OPERAND0_RGB 0x8590
#define GL_OPERAND1_RGB 0x8591
#define GL_OPERAND0_ALPHA 0x8598
#define GL_OPERAND1_ALPHA 0x8599
#define GL_CONSTANT 0x8576
#define GL_PREVIOUS 0x8578
#define GL_PRIMARY_COLOR 0x8577
#define GL_INTERPOLATE 0x8575
#define GL_ADD_SIGNED 0x8574
#define GL_SUBTRACT 0x84E7
#define GL_RGB_SCALE 0x8573
#endif
#ifndef GL_UNSIGNED_SHORT_4_4_4_4
#define GL_UNSIGNED_SHORT_4_4_4_4 0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1 0x8034
#define GL_UNSIGNED_SHORT_5_6_5 0x8363
#endif

typedef void (*PFNGLACTIVETEXTUREPROC)(GLenum texture);
typedef void (*PFNGLCLIENTACTIVETEXTUREPROC)(GLenum texture);

void glActiveTexture(GLenum texture);
void glClientActiveTexture(GLenum texture);
void glFogf(GLenum pname, GLfloat param);
void glFogfv(GLenum pname, const GLfloat *params);
void glScissor(GLint x, GLint y, GLsizei width, GLsizei height);
void glTexParameterf(GLenum target, GLenum pname, GLfloat param);
void glTexEnvf(GLenum target, GLenum pname, GLfloat param);
void glTexEnvfv(GLenum target, GLenum pname, const GLfloat *params);
void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
void glDrawArrays(GLenum mode, GLint first, GLsizei count);
void glDrawArrays(GLenum mode, GLint first, GLsizei count);
void retrofw_VertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr);
void retrofw_TexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr);
#define glVertexPointer retrofw_VertexPointer
#define glTexCoordPointer retrofw_TexCoordPointer
void retrofw_BindTexture(GLenum target, GLuint tex);
#define glBindTexture retrofw_BindTexture
void retrofw_EnableClientState(GLenum cap);
void retrofw_DisableClientState(GLenum cap);
#define glEnableClientState retrofw_EnableClientState
#define glDisableClientState retrofw_DisableClientState
void retrofw_Color4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
#define glColor4f retrofw_Color4f
void glAlphaFunc(GLenum func, GLclampf ref);
void glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
void glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
void glColor4ub(GLubyte r, GLubyte g, GLubyte b, GLubyte a);
void glOrthof(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);
void glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data);
void* retrofw_GetProcAddress(const char* proc);

void retrofw_InitGL(SDL_Surface* screen);
void retrofw_SwapBuffers(SDL_Surface* screen);
void retrofw_ShutdownGL(void);
#ifdef __cplusplus
}
#endif
#endif
#endif
