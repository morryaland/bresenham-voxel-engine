#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "micro_ply.h"
#include "ply2vox.h"
#include "ray.h"


int main(int argc, char **argv)
{
  if (argc < 2)
    return -1;

  SDL_Event e;
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
  SDL_Window *wind = SDL_CreateWindow("", 800, 600, 0);
  SDL_ShowWindow(wind);
  SDL_Surface *sur = SDL_GetWindowSurface(wind);

  SDL_Color *morton;
  int morton_len = read_ply(argv[1], &morton);
  add_morton(morton, morton_len);
  cam_t *cam = get_cam();

  while (1) {
    while (SDL_PollEvent(&e)) {
      switch (e.type) {
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
          return 0;
      }
    }
    const bool *key = SDL_GetKeyboardState(NULL);
    if (key[SDL_SCANCODE_W])
      cam->x++;
    if (key[SDL_SCANCODE_S])
      cam->x--;
    if (key[SDL_SCANCODE_A])
      cam->y++;
    if (key[SDL_SCANCODE_D])
      cam->y--;
    if (key[SDL_SCANCODE_SPACE])
      cam->z++;
    if (key[SDL_SCANCODE_C])
      cam->z--;
    if (key[SDL_SCANCODE_LEFT])
      cam->a += 0.1f;
    if (key[SDL_SCANCODE_RIGHT])
      cam->a -= 0.1f;
    if (key[SDL_SCANCODE_DOWN])
      cam->b -= 0.1f;
    if (key[SDL_SCANCODE_UP])
      cam->b += 0.1f;
    SDL_ClearSurface(sur, 0, 0, 0, 1);
    for (int i = 0; i < 600; i++) {
      for (int j = 0; j < 800; j++) {
        float u = j / 800.0f - 0.5f;
        float v = i / 600.0f - 0.5f;
        SDL_Color p = cast(u, v);
        SDL_WriteSurfacePixel(sur, j, 600 - i, p.r, p.g, p.b, 255);
      }
    }
    SDL_UpdateWindowSurface(wind);
  }

  free(morton);
  return 0;
}
