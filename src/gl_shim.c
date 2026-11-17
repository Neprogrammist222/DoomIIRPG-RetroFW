#define GL_UNSIGNED_SHORT_5_5_5_1 0x8034
typedef float GLclampf;
#include "gl_shim.h"
#include <string.h>
#include <stdio.h>

static ZBuffer *zb = NULL; static int dbg_fc_ref = 0; static int skip_draw = 0;
void retrofw_SetSkipDraw(int skip) { skip_draw = skip; }

void retrofw_InitGL(SDL_Surface* screen) {
    zb = ZB_open(320, 240, ZB_MODE_5R6G5B, 0);
    glInit(zb);
    fprintf(stderr, "SCREEN: %dx%d bpp=%d pitch=%d\n", screen->w, screen->h, screen->format->BitsPerPixel, screen->pitch);
}

void retrofw_SwapBuffers(SDL_Surface* screen) {
#ifndef __mips__
    static int f12_prev = 0, shot_id = 1;
    Uint8 *ks = SDL_GetKeyState(NULL);
    if (ks && ks[SDLK_F12]) {
        if (!f12_prev) {
            char fn[64]; snprintf(fn, sizeof(fn), "/home/deck/shot_%d.bmp", shot_id++);
            SDL_SaveBMP(screen, fn); fprintf(stderr, "SAVED %s\n", fn);
        }
        f12_prev = 1;
    } else f12_prev = 0;
#endif
    if (!zb || !screen) return;
    if (SDL_MUSTLOCK(screen)) SDL_LockSurface(screen);
    ZB_copyFrameBuffer(zb, screen->pixels, screen->pitch);
    if (SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);
    SDL_Flip(screen);
}

void retrofw_ShutdownGL(void) {
    if (zb) {
        glClose();
        ZB_close(zb);
        zb = NULL;
    }
}

void glActiveTexture(GLenum texture) { (void)texture; }
void glClientActiveTexture(GLenum texture) { (void)texture; }
void glFogf(GLenum pname, GLfloat param) { (void)pname; (void)param; }
void glFogfv(GLenum pname, const GLfloat *params) { (void)pname; (void)params; }
void glScissor(GLint x, GLint y, GLsizei width, GLsizei height) { (void)x; (void)y; (void)width; (void)height; }
void glAlphaFunc(GLenum func, GLclampf ref) { (void)func; (void)ref; }
void glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar) {
    GLfloat m[16] = {0};
    m[0] = 2.0f / (float)(right - left);
    m[5] = 2.0f / (float)(top - bottom);
    m[10] = -2.0f / (float)(zFar - zNear);
    m[12] = -(float)((right + left) / (right - left));
    m[13] = -(float)((top + bottom) / (top - bottom));
    m[14] = -(float)((zFar + zNear) / (zFar - zNear));
    m[15] = 1.0f;
    glMultMatrixf(m);
}
void glOrthof(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar) {
    glOrtho((GLdouble)left, (GLdouble)right, (GLdouble)bottom, (GLdouble)top, (GLdouble)zNear, (GLdouble)zFar);
}
void glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data) {
    (void)target; (void)level; (void)internalformat; (void)width; (void)height; (void)border; (void)imageSize; (void)data;
}
void* retrofw_GetProcAddress(const char* proc) {
    if (!strcmp(proc, "glActiveTexture")) return (void*)glActiveTexture;
    if (!strcmp(proc, "glClientActiveTexture")) return (void*)glClientActiveTexture;
    return NULL;
}
void glTexParameterf(GLenum target, GLenum pname, GLfloat param) { glTexParameteri(target, pname, (GLint)param); }
void glTexEnvf(GLenum target, GLenum pname, GLfloat param) { glTexEnvi(target, pname, (GLint)param); }
void glTexEnvfv(GLenum target, GLenum pname, const GLfloat *params) { (void)target; (void)pname; (void)params; }

static float cur_r = 1.0f, cur_g = 1.0f, cur_b = 1.0f, cur_alpha = 1.0f;
#undef glColor4f
static const float *cur_tc = NULL;
void retrofw_Color4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a) { cur_r = r; cur_g = g; cur_b = b; cur_alpha = a; glColor4f(r, g, b, a); }
void glColor4ub(GLubyte r, GLubyte g, GLubyte b, GLubyte a) { retrofw_Color4f(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f); }

static GLuint cur_bound_tex = 0, fade_tex = 999999;
#undef glBindTexture
void retrofw_BindTexture(GLenum target, GLuint tex) { if (target == GL_TEXTURE_2D) cur_bound_tex = tex; glBindTexture(target, tex); }
void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels) {
    (void)internalformat; (void)border; (void)format;
    static uint8_t buf[256 * 256 * 3];
    if (!pixels || width <= 0 || height <= 0) return;

    for (int y = 0; y < 256; y++) {
        int sy = (y * height) / 256;
        for (int x = 0; x < 256; x++) {
            int sx = (x * width) / 256;
            int di = (y * 256 + x) * 3;
            int si = sy * width + sx;
            uint8_t r = 0, g = 0, b = 0;
            if (type == 0x8363) {
                uint16_t px = ((const uint16_t*)pixels)[si];
                r = ((px >> 11) & 0x1F) * 255 / 31;
                g = ((px >> 5) & 0x3F) * 255 / 63;
                b = (px & 0x1F) * 255 / 31;
                if (r == 255 && g == 0 && b == 255) g = 1;
            } else if (type == 0x8034) {
                uint16_t px = ((const uint16_t*)pixels)[si];
                if ((px & 1) == 0) { r = 255; g = 0; b = 255; }
                else {
                    r = ((px >> 11) & 0x1F) * 255 / 31;
                    g = ((px >> 6) & 0x1F) * 255 / 31;
                    b = ((px >> 1) & 0x1F) * 255 / 31;
                    if (r == 255 && g == 0 && b == 255) g = 1;
                }
            } else if (type == 0x8366 || type == 0x8367) {
                uint16_t px = ((const uint16_t*)pixels)[si];
                if ((px & 1) == 0) { r = 255; g = 0; b = 255; }
                else {
                    r = ((px >> 11) & 0x1F) * 255 / 31;
                    g = ((px >> 6) & 0x1F) * 255 / 31;
                    b = ((px >> 1) & 0x1F) * 255 / 31;
                    if (r == 255 && g == 0 && b == 255) g = 1;
                }
            } else if (type == GL_UNSIGNED_BYTE) { fade_tex = cur_bound_tex;
                const uint8_t *px = ((const uint8_t*)pixels) + si * 4;
                if (px[3] < 128) { r = 255; g = 0; b = 255; }
                else { r = px[0]; g = px[1]; b = px[2]; if (r == 255 && g == 0 && b == 255) g = 1; }
            }
            buf[di] = r; buf[di + 1] = g; buf[di + 2] = b;
        }
    }
    tgl_TexImage2D(target, level, 3, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, buf);
}

static const float *cur_vp = NULL;
static int cur_vp_size = 3;
static int cur_vp_stride = 3;

static int tc_enabled = 0;
static int cur_tc_stride = 2;

#undef glEnableClientState
#undef glDisableClientState
void retrofw_EnableClientState(GLenum cap) {
    if (cap == GL_TEXTURE_COORD_ARRAY) tc_enabled = 1;
    glEnableClientState(cap);
}
void retrofw_DisableClientState(GLenum cap) {
    if (cap == GL_TEXTURE_COORD_ARRAY) tc_enabled = 0;
    glDisableClientState(cap);
}
void retrofw_VertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr) {
    cur_tc = NULL;
    (void)type;
    cur_vp = (const float*)ptr;
    cur_vp_size = size;
    cur_vp_stride = stride ? stride / (int)sizeof(float) : size;
}
void retrofw_TexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr) {
    (void)type;
    cur_tc = (const float*)ptr;
    cur_tc_stride = stride ? stride / (int)sizeof(float) : size;
}
static void emit_vert(int idx) {
    if (cur_tc) glTexCoord2f(cur_tc[idx * cur_tc_stride], cur_tc[idx * cur_tc_stride + 1]);
    float x = cur_vp[idx * cur_vp_stride];
    float y = cur_vp[idx * cur_vp_stride + 1];
    float z = (cur_vp_size >= 3) ? cur_vp[idx * cur_vp_stride + 2] : 0.0f;
    if (cur_vp_size >= 4) {
        float w = cur_vp[idx * cur_vp_stride + 3];
        if (w != 0.0f && w != 1.0f) { x /= w; y /= w; z /= w; }
    }
    glVertex3f(x, y, z);
}

void glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices) {
    if (skip_draw) return;
    if (!cur_vp || count <= 0) return;
    
    if (!cur_tc && cur_alpha < 0.95f) return;
    
    glDisable(GL_CULL_FACE);
    if (cur_tc) { glEnable(GL_TEXTURE_2D); glColor4f(cur_r, cur_g, cur_b, 1.0f); } else { glDisable(GL_TEXTURE_2D); }
    glBegin(mode);
    if (type == GL_UNSIGNED_SHORT) {
        const uint16_t *idx = (const uint16_t *)indices;
        for (GLsizei i = 0; i < count; i++) emit_vert(idx[i]);
    } else if (type == GL_UNSIGNED_INT) {
        const uint32_t *idx = (const uint32_t *)indices;
        for (GLsizei i = 0; i < count; i++) emit_vert(idx[i]);
    }
    glEnd();
}


void glDrawArrays(GLenum mode, GLint first, GLsizei count) {
    if (!cur_vp || count <= 0) return;
    
    if (!cur_tc && cur_alpha < 0.95f) return;
    
    glDisable(GL_CULL_FACE);
    if (cur_tc) { glEnable(GL_TEXTURE_2D); glColor4f(cur_r, cur_g, cur_b, 1.0f); } else { glDisable(GL_TEXTURE_2D); }
    if (mode == GL_TRIANGLE_STRIP && count >= 3) {
        glBegin(GL_TRIANGLES);
        for (GLsizei i = 0; i < count - 2; i++) {
            if ((i & 1) == 0) { emit_vert(first + i); emit_vert(first + i + 1); emit_vert(first + i + 2); }
            else { emit_vert(first + i + 1); emit_vert(first + i); emit_vert(first + i + 2); }
        }
        glEnd();
    } else if (mode == GL_TRIANGLE_FAN && count >= 3) {
        glBegin(GL_TRIANGLES);
        for (GLsizei i = 1; i < count - 1; i++) {
            emit_vert(first); emit_vert(first + i); emit_vert(first + i + 1);
        }
        glEnd();
    } else {
        glBegin(mode);
        for (GLsizei i = 0; i < count; i++) emit_vert(first + i);
        glEnd();
    }
}
