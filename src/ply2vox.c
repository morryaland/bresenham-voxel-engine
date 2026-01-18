#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "micro_ply.h"
#include "ply2vox.h"

typedef float float3[3];

long read_ply(char *path, SDL_Color **morton)
{
  int vertex_count = 0;
  vertex_t *vertexs = NULL;
  void  *data;
  size_t size;
  FILE  *fp;
  fp = fopen(path, "rb");
  fseek(fp, 0L, SEEK_END);
  size = ftell(fp);
  rewind(fp);
  data = malloc(size);
  fread(data, size, 1, fp);
  fclose(fp);
  
  vertex_count = read_pc(data, size, &vertexs);
  free(data);

  long max = 0;
  long *indexs = malloc(vertex_count * sizeof(long));
  for (int i = 0; i < vertex_count; i++) {
    unsigned int x = vertexs[i].x;
    unsigned int y = vertexs[i].y;
    unsigned int z = vertexs[i].z;
    indexs[i] = zorder(x, y, z);
    if (max < indexs[i])
        max = indexs[i];
  }
  *morton = calloc(max + 1, sizeof(int));
  for (int i = 0; i < vertex_count; i++) {
    (*morton)[indexs[i]] = (SDL_Color){.r = vertexs[i].r, .g = vertexs[i].g, .b = vertexs[i].b, .a = UINT8_MAX};
  }
  return max + 1;
}

int read_pc(void *data, size_t size, vertex_t **vertex)
{
  int vertex_c;
  ply_file_t file;
  ply_read(data, size, &file);

  float   fzero = 0;
  uint8_t white = 255;
  ply_map_t map_verts[] = {
    { PLY_PROP_POSITION_X,  ply_prop_decimal, sizeof(float), 0,  &fzero },
    { PLY_PROP_POSITION_Y,  ply_prop_decimal, sizeof(float), 4,  &fzero },
    { PLY_PROP_POSITION_Z,  ply_prop_decimal, sizeof(float), 8,  &fzero },
    { PLY_PROP_COLOR_R,     ply_prop_uint,    sizeof(uint8_t), 12, &white },
    { PLY_PROP_COLOR_G,     ply_prop_uint,    sizeof(uint8_t), 13, &white },
    { PLY_PROP_COLOR_B,     ply_prop_uint,    sizeof(uint8_t), 14, &white }, };
  ply_convert(&file, PLY_ELEMENT_VERTICES, map_verts, sizeof(map_verts)/sizeof(map_verts[0]), sizeof(vertex_t), (void**)vertex, &vertex_c);
  ply_free(&file);
  return vertex_c;
}
