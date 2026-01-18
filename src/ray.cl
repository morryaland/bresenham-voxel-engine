#define max3(x, y, z) x > y ? x > z ? x : z : y > z ? y : z

inline unsigned long inflation(unsigned x)
{
  x = (x | (x << 16)) & 0x030000FF;
  x = (x | (x <<  8)) & 0x0300F00F;
  x = (x | (x <<  4)) & 0x030C30C3;
  x = (x | (x <<  2)) & 0x09249249;
  return x;
}

inline unsigned long zorder(unsigned int x, unsigned int y, unsigned int z)
{
  return inflation(x) | (inflation(y) << 1) | (inflation(z) << 2);
}

uchar4 plotLine3d(__constant uchar4 *morton, int morton_c, int x0, int y0, int z0, int x1, int y1, int z1)
{
   int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
   int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1; 
   int dz = abs(z1-z0), sz = z0<z1 ? 1 : -1; 
   int dm = max3(dx,dy,dz), i = dm;
   for(x1 = y1 = z1 = i/2; i-- >= 0; ) {
      int index = zorder(x0, y0, z0);
      if (index < morton_c && index >= 0 && morton[index].w)
         return morton[index];
      x1 -= dx; if (x1 < 0) { x1 += dm; x0 += sx; } 
      y1 -= dy; if (y1 < 0) { y1 += dm; y0 += sy; } 
      z1 -= dz; if (z1 < 0) { z1 += dm; z0 += sz; } 
   }
   return (uchar4)(0, 0, 0, UCHAR_MAX);
}

__kernel void raycast(__write_only image2d_t screen,
                      __constant uchar4 *morton,
                      __constant float *tm,
                      const int morton_c,
                      const int ray_dist,
                      const int x,
                      const int y,
                      const int z)
{
  float u = (float)get_global_id(0) / get_global_size(0) - 0.5f;
  float v = (float)get_global_id(1) / get_global_size(1) - 0.5f;
  int dx, dy, dz;
  dx = dy = dz = ray_dist;
  dx *= u * tm[0] - v * tm[1] - tm[2], dx += x;
  dy *= u * tm[3] + v * tm[4] + tm[5], dy += y;
  dz *= v * tm[6] - tm[7], dz += z;
  write_imagei(screen, (int2)(get_global_id(0), get_global_size(1) - get_global_id(1)), convert_int4(plotLine3d(morton, morton_c, x, y, z, dx, dy, dz)));
}
