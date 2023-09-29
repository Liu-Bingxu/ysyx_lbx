#define SDL_malloc  malloc
#define SDL_free    free
#define SDL_realloc realloc

#define SDL_STBIMAGE_IMPLEMENTATION
#include "SDL_stbimage.h"
#include "stdio.h"

SDL_Surface* IMG_Load_RW(SDL_RWops *src, int freesrc) {
  assert(src->type == RW_TYPE_MEM);
  assert(freesrc == 0);
  return NULL;
}

SDL_Surface* IMG_Load(const char *filename) {
    FILE *img_fp = NULL;
    img_fp = fopen(filename, "rb");
    assert(img_fp);
    fseek(img_fp, 0, SEEK_END);
    size_t size = ftell(img_fp);
    fseek(img_fp, 0, SEEK_SET);
    void *buf = NULL;
    buf = malloc(size);
    assert(buf);
    int len=fread(buf,size,1,img_fp);
    assert(len == 1);
    SDL_Surface *face = NULL;
    face=STBIMG_LoadFromMemory(buf, size);
    assert(face);
    fclose(img_fp);
    img_fp = NULL;
    free(buf);
    buf = NULL;
    return face;
}

int IMG_isPNG(SDL_RWops *src) {
  return 0;
}

SDL_Surface* IMG_LoadJPG_RW(SDL_RWops *src) {
  return IMG_Load_RW(src, 0);
}

char *IMG_GetError() {
  return "Navy does not support IMG_GetError()";
}
