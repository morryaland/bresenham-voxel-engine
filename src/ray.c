#include <SDL3/SDL.h>
#include <stdlib.h>
#include <math.h>
#include "ply2vox.h"
#include "ray.h"

typedef struct { SDL_Color **morton; int *morton_len; int morton_c; } morton_list_t;

static cam_t cam;
static morton_list_t morton_list;

#define max(x, y, z) x > y ? x > z ? x : z : y > z ? y : z

void add_morton(SDL_Color *morton, int morton_len)
{
  morton_list.morton = realloc(morton_list.morton, (morton_list.morton_c + 1) * sizeof(int*));
  morton_list.morton[morton_list.morton_c] = morton;
  morton_list.morton_len = realloc(morton_list.morton_len, (morton_list.morton_c + 1) * sizeof(int));
  morton_list.morton_len[morton_list.morton_c] = morton_len;
  morton_list.morton_c++;
}

SDL_Color fog_color(SDL_Color src, float v)
{
  src.r += (UINT8_MAX - src.r) * v;
  src.g += (UINT8_MAX - src.g) * v;
  src.b += (UINT8_MAX - src.b) * v;
  return src;
}

SDL_Color plotLine3d(int x0, int y0, int z0, int x1, int y1, int z1)
{
   float fog = 0;
   int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
   int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1; 
   int dz = abs(z1-z0), sz = z0<z1 ? 1 : -1; 
   int dm = max(dx,dy,dz), i = SDL_min(dm, MAXDIST); /* maximum difference */
   for(x1 = y1 = z1 = i/2; i-- >= 0; ) {  /* loop */
      int index = zorder(x0, y0, z0);
      if (index < morton_list.morton_len[0] && index >= 0 && morton_list.morton[0][index].a)
         return fog_color(morton_list.morton[0][index], fog);
      x1 -= dx; if (x1 < 0) { x1 += dm; x0 += sx; } 
      y1 -= dy; if (y1 < 0) { y1 += dm; y0 += sy; } 
      z1 -= dz; if (z1 < 0) { z1 += dm; z0 += sz; } 
      fog += 1.0f / MAXDIST;
   }
   return (SDL_Color){UINT8_MAX, UINT8_MAX, UINT8_MAX, UINT8_MAX};
}

SDL_Color cast(float u, float v)
{
  int dx, dy, dz;
  dx = dy = dz = RAYDIST;
  dx *= u * cosf(cam.a) - sinf(cam.a) * (v * cosf(cam.b) + sinf(cam.b));
  dy *= cosf(cam.a) * (v * cosf(cam.b) + sinf(cam.b)) + u * sinf(cam.a);
  dz *= v * sinf(cam.b) - cosf(cam.b);
  dx += cam.x;
  dy += cam.y;
  dz += cam.z;
  return plotLine3d(cam.x, cam.y, cam.z, dx, dy, dz);
}

cam_t *get_cam()
{
  return &cam;
}
