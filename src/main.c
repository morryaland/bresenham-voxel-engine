#define CL_TARGET_OPENCL_VERSION 120
#include <CL/opencl.h>
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <math.h>
#include "micro_ply.h"
#include "ply2vox.h"
#include "ray.h"

struct cam { int x, y, z; float a, b; } cam;

int main(int argc, char **argv)
{
  SDL_Event e;
  cl_platform_id platform;
  cl_device_id device;
  float tm[8];
  cl_int ret;
  int raydist = 100;

  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
  SDL_Window *wind = SDL_CreateWindow("", 800, 600, 0);
  SDL_ShowWindow(wind);
  SDL_Surface *sur = SDL_GetWindowSurface(wind);
  SDL_Surface *screen = SDL_CreateSurface(sur->w, sur->h, SDL_PIXELFORMAT_ABGR8888);
  assert(screen != NULL);

  char c;
  while ((c = getopt(argc, argv, "d:")) > 0) {
    switch (c) {
      case 'd':
        raydist = atoi(optarg);
        break;
    }
  }

  SDL_Color *morton;
  int morton_c = read_ply(argv[argc - 1], &morton);

  ret = clGetPlatformIDs(1, &platform, NULL);
  assert(ret == CL_SUCCESS && "get platform error");
  ret = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, NULL);
  assert(ret == CL_SUCCESS && "get device error");
  cl_context clctx = clCreateContext(NULL, 1, &device, NULL, NULL, &ret);
  assert(ret == CL_SUCCESS && "create cotext error");
  cl_command_queue queue = clCreateCommandQueue(clctx, device, 0, &ret);
  assert(ret == CL_SUCCESS && "create queue error");
  
  const cl_image_desc img_desc = {
    .image_type = CL_MEM_OBJECT_IMAGE2D,
    .image_width = screen->w,
    .image_height = screen->h
  };
  const cl_image_format img_format = {
    .image_channel_data_type = CL_UNSIGNED_INT8,
    .image_channel_order = CL_RGBA
  };

  cl_mem clscreen = clCreateImage(clctx, CL_MEM_WRITE_ONLY, &img_format, &img_desc, NULL, &ret);
  assert(ret == CL_SUCCESS && "create clscreen error");
  cl_mem clmorton = clCreateBuffer(clctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, morton_c * sizeof(SDL_Color), morton, &ret);
  assert(ret == CL_SUCCESS && "create clmorton errror");
  cl_mem cltm = clCreateBuffer(clctx, CL_MEM_READ_ONLY, sizeof(tm), NULL, &ret);
  assert(ret == CL_SUCCESS && "create cltm error");

  cl_program prog = clCreateProgramWithSource(clctx, 1, (const char*[]){ray_cl}, (const size_t[]){ray_cl_len}, &ret);
  assert(ret == CL_SUCCESS && "create program error");
  cl_int err = clBuildProgram(prog, 1, &device, NULL, NULL, NULL);
  if (err != CL_SUCCESS) {
    size_t logsize;
    clGetProgramBuildInfo(prog, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &logsize);
    char *logbuf = malloc(logsize);
    clGetProgramBuildInfo(prog, device, CL_PROGRAM_BUILD_LOG, logsize, logbuf, NULL);
    puts(logbuf);
  }
  cl_kernel kern = clCreateKernel(prog, "raycast", &ret);
  assert(ret == CL_SUCCESS && "create kernel error");
  clSetKernelArg(kern, 0, sizeof(cl_mem), &clscreen);
  clSetKernelArg(kern, 1, sizeof(cl_mem), &clmorton);
  clSetKernelArg(kern, 2, sizeof(cl_mem), &cltm);
  clSetKernelArg(kern, 3, sizeof(int), &morton_c);
  clSetKernelArg(kern, 4, sizeof(int), &raydist);
  clSetKernelArg(kern, 5, sizeof(int), &cam.x);
  clSetKernelArg(kern, 6, sizeof(int), &cam.y);
  clSetKernelArg(kern, 7, sizeof(int), &cam.z);

  while (1) {
    while (SDL_PollEvent(&e)) {
      switch (e.type) {
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
          return 0;
      }
    }
    const bool *key = SDL_GetKeyboardState(NULL);
    if (key[SDL_SCANCODE_W])
      cam.x++, clSetKernelArg(kern, 5, sizeof(int), &cam.x);
    if (key[SDL_SCANCODE_S])
      cam.x--, clSetKernelArg(kern, 5, sizeof(int), &cam.x);
    if (key[SDL_SCANCODE_A])
      cam.y++, clSetKernelArg(kern, 6, sizeof(int), &cam.y);
    if (key[SDL_SCANCODE_D])
      cam.y--, clSetKernelArg(kern, 6, sizeof(int), &cam.y);
    if (key[SDL_SCANCODE_SPACE])
      cam.z++, clSetKernelArg(kern, 7, sizeof(int), &cam.z);
    if (key[SDL_SCANCODE_C])
      cam.z--, clSetKernelArg(kern, 7, sizeof(int), &cam.z);
    if (key[SDL_SCANCODE_LEFT])
      cam.a += 0.1f;
    if (key[SDL_SCANCODE_RIGHT])
      cam.a -= 0.1f;
    if (key[SDL_SCANCODE_DOWN])
      cam.b -= 0.1f;
    if (key[SDL_SCANCODE_UP])
      cam.b += 0.1f;
    tm[0] = cosf(cam.a), tm[1] = sinf(cam.a) * cosf(cam.b), tm[2] = sinf(cam.a) * sinf(cam.b), tm[3] = sinf(cam.a), tm[4] = cosf(cam.a) * cosf(cam.b), tm[5] = cosf(cam.a) * sinf(cam.b), tm[6] = sinf(cam.b), tm[7] = cosf(cam.b);
    ret = clEnqueueWriteBuffer(queue, cltm, true, 0, sizeof(tm), tm, 0, NULL, NULL);
    assert(ret == CL_SUCCESS && "write tm error");
    ret = clEnqueueNDRangeKernel(queue, kern, 2, NULL, (size_t[]){screen->w, screen->h}, NULL, 0, NULL, NULL);
    assert(ret == CL_SUCCESS && "run kernel error");
    ret = clEnqueueReadImage(queue, clscreen, true, (const size_t[]){ 0, 0, 0 }, (const size_t[]){ screen->w, screen->h, 1 }, 0, 0, screen->pixels, 0, NULL, NULL);
    assert(ret == CL_SUCCESS && "read screen from clscreen error");
    SDL_BlitSurface(screen, NULL, sur, NULL);
    SDL_UpdateWindowSurface(wind);
  }

  free(morton);
  return 0;
}
