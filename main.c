#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <emmintrin.h>
#include <mmintrin.h>
#include <smmintrin.h>


typedef struct
{ 
  char     type[2];
  size_t   size;
  uint16_t reserved1; 
  uint16_t reserved2; 
  uint32_t offBits;
} __attribute__((packed)) BITMAPFILEHEADER;

typedef struct 
{
  size_t   size; 
  size_t   width; 
  size_t   height; 
  uint16_t planes; 
  uint16_t bitCount; 
  uint32_t compression; 
  uint32_t sizeImage; 
  size_t   xPelsPerMeter; 
  size_t   yPelsPerMeter; 
  uint32_t clrUsed; 
  uint32_t clrImportant; 
} __attribute__((packed)) BITMAPINFOHEADER;

typedef struct Bitmap
{
  size_t   width;
  size_t   height;
  size_t   channels;
  uint8_t *ptr;
} *Bitmap;


void free_bitmap(Bitmap bmp)
{
  free(bmp->ptr);
  free(bmp);
}

Bitmap load_bitmap(char *path)
{
  Bitmap  res;
  FILE   *file;
  size_t  rowsize, length, i;
  BITMAPFILEHEADER fileheader;
  BITMAPINFOHEADER infoheader;

  file = fopen(path, "rb");
  length = fread(&fileheader, 1, sizeof(fileheader), file);
  length = fread(&infoheader, 1, sizeof(infoheader), file);
  rowsize = (infoheader.width * 3 + 3) >> 2 << 2;

  res = (Bitmap)calloc(1, sizeof(struct Bitmap));
  res->channels = 3;
  res->width = infoheader.width;
  res->height = abs(infoheader.height);
  res->ptr = malloc(res->width * res->height * res->channels);

  fseek(file, sizeof(fileheader) + infoheader.size, SEEK_SET);
  for (i = 0; i < res->height; ++i)
  {
    length = fread(&res->ptr[i * res->width * res->channels], 1, res->width * res->channels, file);
    fseek(file, rowsize - length, SEEK_CUR);
  }

  fclose(file);
  return res;
}

void save_bitmap(Bitmap bmp, char *path)
{
  FILE   *file;
  size_t  rowsize, length, i;
  BITMAPFILEHEADER fileheader = {"BM", 0, 0, 0, 54};
  BITMAPINFOHEADER infoheader = {40, 0, 0, 1, 24, 0, 0, 1, 1, 0, 0};

  rowsize = (bmp->width * bmp->channels + 3) >> 2 << 2;
  length = rowsize * bmp->height;

  fileheader.size = length + 54;
  infoheader.width = bmp->width;
  infoheader.height = bmp->height;
  infoheader.sizeImage = length;
  infoheader.bitCount = bmp->channels * 8;

  file = fopen(path, "w");
  fwrite(&fileheader, 1, sizeof(BITMAPFILEHEADER), file);
  fwrite(&infoheader, 1, sizeof(BITMAPINFOHEADER), file);
  for (i = 0; i < bmp->height; i++)
  {
    fwrite(&bmp->ptr[i * bmp->width * bmp->channels], 1, rowsize, file);
  }
  fclose(file);
}


double linear_easing(double x) { return x; }
double square_easing(double x) { return x * x; }
double cubic_easing(double x) { return x * x * x; }

#include "scale.c"
#include "linear.c"
#include "inverse.c"


int main(int argc, char **argv)
{
  Bitmap  bmp, res;
  float   time;
  clock_t start;
  int     i, width, height, times = 50;

  if (argc < 4)
  {
    printf("usage: %s input.bmp output.bmp <width> <height> <times>\n", argv[0]);
    return 0;
  }

  width = atoi(argv[3]);
  height = atoi(argv[4]);
  if (argc > 5)
  {
    times = atoi(argv[5]);
  }

  bmp = load_bitmap(argv[1]);

  start = clock();
  for (i = 0; i < times; ++i)
  {
    res = fast_inverse_resize_bitmap(bmp, width, height);
    if (i != times -1)
    {
      free_bitmap(res);
    }
  }
  time = (float)(clock() - start) / CLOCKS_PER_SEC;

  save_bitmap(res, argv[2]);

  free_bitmap(bmp);
  printf("%d times completed in %f sec\n", times, time);

  return 0;
}
