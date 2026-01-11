#ifndef __RAY_H__
#define __RAY_H__

#include <SDL3/SDL.h>

#define MAXDIST 100
#define RAYDIST 100

typedef struct { int x, y, z; float a, b; } cam_t;

cam_t *get_cam();

void add_morton(SDL_Color *morton, int morton_len);

SDL_Color cast(float u, float v);

#endif
