#ifndef __PLY2VOX_H__
#define __PLY2VOX_H__

#include <SDL3/SDL.h>
#include <immintrin.h>
#include <inttypes.h>
#include <stdlib.h>

#if defined(__BMI2__) || defined(__AVX2__)
#define BMI_3D_X_MASK 0x9249249249249249
#define BMI_3D_Y_MASK 0x2492492492492492
#define BMI_3D_Z_MASK 0x4924924924924924
#define zorder(x, y, z) _pdep_u64(x, BMI_3D_X_MASK) | _pdep_u64(y, BMI_3D_Y_MASK) | _pdep_u64(z, BMI_3D_Z_MASK)
#else
inline unsigned long inflation(unsigned x){
x = (x | (x << 16)) & 0x030000FF;
x = (x | (x <<  8)) & 0x0300F00F;
x = (x | (x <<  4)) & 0x030C30C3;
x = (x | (x <<  2)) & 0x09249249;
return x;
}
inline unsigned long zorder(unsigned x, unsigned y, unsigned z)
{
  return inflation(x) | (inflation(y) << 1) | (inflation(z) << 2);
}
#endif

typedef struct {
  float x, y, z;
  uint8_t r, g, b;
} vertex_t;

long read_ply(char *path, SDL_Color **morton);

int read_pc(void *data, size_t size, vertex_t **vertex);

#endif
