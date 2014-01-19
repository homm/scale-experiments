#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <malloc.h>


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

typedef struct
{
  size_t   width;
  size_t   height;
  size_t   channels;
  uint8_t *ptr;
} Bitmap;


void free_bitmap(Bitmap *bmp)
{
  free(bmp->ptr);
  free(bmp);
}

Bitmap *load_bitmap(char *path)
{
  Bitmap *res;
  FILE   *file;
  size_t  rowsize, length, i;
  BITMAPFILEHEADER fileheader;
  BITMAPINFOHEADER infoheader;

  file = fopen(path, "rb");
  length = fread(&fileheader, 1, sizeof(fileheader), file);
  length = fread(&infoheader, 1, sizeof(infoheader), file);
  rowsize = (infoheader.width * 3 + 3) >> 2 << 2;

  res = (Bitmap *)calloc(1, sizeof(Bitmap));
  res->channels = 3;
  res->width = infoheader.width;
  res->height = infoheader.height;
  res->ptr = malloc(res->width * res->height * res->channels);

  for (i = 0; i < res->height; ++i)
  {
    length = fread(&res->ptr[i * res->width * res->channels], 1, res->width * res->channels, file);
    fseek(file, rowsize - length, SEEK_CUR);
  }

  fclose(file);
  return res;
}

void save_bitmap(char *path, Bitmap *bmp)
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

Bitmap *scale_bitmap(Bitmap *bmp, int scale)
{
  Bitmap  *res;
  uint8_t *src1, *src2, *src3, *src4, *src5, *src6, *src7, *src8, *dst;
  size_t   srcrow, dstrow, cc;
  size_t   x, y, c;

  res = (Bitmap *)calloc(1, sizeof(Bitmap));
  res->width = bmp->width / scale;
  res->height = bmp->height / scale;
  res->channels = bmp->channels;
  res->ptr = malloc(res->width * res->height * res->channels);

  cc = bmp->channels;
  dstrow = res->width * cc;
  srcrow = bmp->width * cc;

  #define PIXEL_LOOP_2X(BODY) \
    for (y = 0; y < res->height; y++)\
    {\
      src1 = &bmp->ptr[(y*2 + 0) * srcrow];\
      src2 = &bmp->ptr[(y*2 + 1) * srcrow];\
      dst = &res->ptr[y * dstrow];\
      for (x = 0; x < dstrow; x += cc)\
      {\
        BODY\
      }\
    }

  #define PIXEL_LOOP_3X(BODY) \
    for (y = 0; y < res->height; y++)\
    {\
      src1 = &bmp->ptr[(y*3 + 0) * srcrow];\
      src2 = &bmp->ptr[(y*3 + 1) * srcrow];\
      src3 = &bmp->ptr[(y*3 + 2) * srcrow];\
      dst = &res->ptr[y * dstrow];\
      for (x = 0; x < dstrow; x += cc)\
      {\
        BODY\
      }\
    }

  #define PIXEL_LOOP_4X(BODY) \
    for (y = 0; y < res->height; y++)\
    {\
      src1 = &bmp->ptr[(y*4 + 0) * srcrow];\
      src2 = &bmp->ptr[(y*4 + 1) * srcrow];\
      src3 = &bmp->ptr[(y*4 + 2) * srcrow];\
      src4 = &bmp->ptr[(y*4 + 3) * srcrow];\
      dst = &res->ptr[y * dstrow];\
      for (x = 0; x < dstrow; x += cc)\
      {\
        BODY\
      }\
    }

  #define PIXEL_LOOP_6X(BODY) \
    for (y = 0; y < res->height; y++)\
    {\
      src1 = &bmp->ptr[(y*6 + 0) * srcrow];\
      src2 = &bmp->ptr[(y*6 + 1) * srcrow];\
      src3 = &bmp->ptr[(y*6 + 2) * srcrow];\
      src4 = &bmp->ptr[(y*6 + 3) * srcrow];\
      src5 = &bmp->ptr[(y*6 + 4) * srcrow];\
      src6 = &bmp->ptr[(y*6 + 5) * srcrow];\
      dst = &res->ptr[y * dstrow];\
      for (x = 0; x < dstrow; x += cc)\
      {\
        BODY\
      }\
    }

  #define PIXEL_LOOP_8X(BODY) \
    for (y = 0; y < res->height; y++)\
    {\
      src1 = &bmp->ptr[(y*8 + 0) * srcrow];\
      src2 = &bmp->ptr[(y*8 + 1) * srcrow];\
      src3 = &bmp->ptr[(y*8 + 2) * srcrow];\
      src4 = &bmp->ptr[(y*8 + 3) * srcrow];\
      src5 = &bmp->ptr[(y*8 + 4) * srcrow];\
      src6 = &bmp->ptr[(y*8 + 5) * srcrow];\
      src7 = &bmp->ptr[(y*8 + 6) * srcrow];\
      src8 = &bmp->ptr[(y*8 + 7) * srcrow];\
      dst = &res->ptr[y * dstrow];\
      for (x = 0; x < dstrow; x += cc)\
      {\
        BODY\
      }\
    }

  #define CHANEL_COMPUTION_2X(c, cc) \
    dst[x + c] = (src1[x*2 + c] + src1[x*2 + c + cc] + \
                  src2[x*2 + c] + src2[x*2 + c + cc]) >> 2

  #define CHANEL_COMPUTION_3X(c, cc) \
    dst[x + c] = (src1[x*3 + c] + src1[x*3 + c + cc] + src1[x*3 + c + cc*2] +\
                  src2[x*3 + c] + src2[x*3 + c + cc] + src2[x*3 + c + cc*2] +\
                  src3[x*3 + c] + src3[x*3 + c + cc] + src3[x*3 + c + cc*2]) / 9

  #define CHANEL_COMPUTION_4X(c, cc) \
    dst[x + c] = (src1[x*4 + c] + src1[x*4 + c + cc] + src1[x*4 + c + cc*2] + src1[x*4 + c + cc*3] +\
                  src2[x*4 + c] + src2[x*4 + c + cc] + src2[x*4 + c + cc*2] + src2[x*4 + c + cc*3] +\
                  src3[x*4 + c] + src3[x*4 + c + cc] + src3[x*4 + c + cc*2] + src3[x*4 + c + cc*3] +\
                  src4[x*4 + c] + src4[x*4 + c + cc] + src4[x*4 + c + cc*2] + src4[x*4 + c + cc*3]) >> 4

  #define CHANEL_COMPUTION_6X(c, cc) \
    dst[x + c] = (src1[x*6 + c] + src1[x*6 + c + cc] + src1[x*6 + c + cc*2] + src1[x*6 + c + cc*3] +\
                  src1[x*6 + c + cc*4] + src1[x*6 + c + cc*5] +\
                  src2[x*6 + c] + src2[x*6 + c + cc] + src2[x*6 + c + cc*2] + src2[x*6 + c + cc*3] +\
                  src2[x*6 + c + cc*4] + src2[x*6 + c + cc*5] +\
                  src3[x*6 + c] + src3[x*6 + c + cc] + src3[x*6 + c + cc*2] + src3[x*6 + c + cc*3] +\
                  src3[x*6 + c + cc*4] + src3[x*6 + c + cc*5] +\
                  src4[x*6 + c] + src4[x*6 + c + cc] + src4[x*6 + c + cc*2] + src4[x*6 + c + cc*3] +\
                  src4[x*6 + c + cc*4] + src4[x*6 + c + cc*5] +\
                  src5[x*6 + c] + src5[x*6 + c + cc] + src5[x*6 + c + cc*2] + src5[x*6 + c + cc*3] +\
                  src5[x*6 + c + cc*4] + src5[x*6 + c + cc*5] +\
                  src6[x*6 + c] + src6[x*6 + c + cc] + src6[x*6 + c + cc*2] + src6[x*6 + c + cc*3] +\
                  src6[x*6 + c + cc*4] + src6[x*6 + c + cc*5]) / 36

  #define CHANEL_COMPUTION_8X(c, cc) \
    dst[x + c] = (src1[x*8 + c] + src1[x*8 + c + cc] + src1[x*8 + c + cc*2] + src1[x*8 + c + cc*3] +\
                  src1[x*8 + c + cc*4] + src1[x*8 + c + cc*5] + src1[x*8 + c + cc*6] + src1[x*8 + c + cc*7] +\
                  src2[x*8 + c] + src2[x*8 + c + cc] + src2[x*8 + c + cc*2] + src2[x*8 + c + cc*3] +\
                  src2[x*8 + c + cc*4] + src2[x*8 + c + cc*5] + src2[x*8 + c + cc*6] + src2[x*8 + c + cc*7] +\
                  src3[x*8 + c] + src3[x*8 + c + cc] + src3[x*8 + c + cc*2] + src3[x*8 + c + cc*3] +\
                  src3[x*8 + c + cc*4] + src3[x*8 + c + cc*5] + src3[x*8 + c + cc*6] + src3[x*8 + c + cc*7] +\
                  src4[x*8 + c] + src4[x*8 + c + cc] + src4[x*8 + c + cc*2] + src4[x*8 + c + cc*3] +\
                  src4[x*8 + c + cc*4] + src4[x*8 + c + cc*5] + src4[x*8 + c + cc*6] + src4[x*8 + c + cc*7] +\
                  src5[x*8 + c] + src5[x*8 + c + cc] + src5[x*8 + c + cc*2] + src5[x*8 + c + cc*3] +\
                  src5[x*8 + c + cc*4] + src5[x*8 + c + cc*5] + src5[x*8 + c + cc*6] + src5[x*8 + c + cc*7] +\
                  src6[x*8 + c] + src6[x*8 + c + cc] + src6[x*8 + c + cc*2] + src6[x*8 + c + cc*3] +\
                  src6[x*8 + c + cc*4] + src6[x*8 + c + cc*5] + src6[x*8 + c + cc*6] + src6[x*8 + c + cc*7] +\
                  src7[x*8 + c] + src7[x*8 + c + cc] + src7[x*8 + c + cc*2] + src7[x*8 + c + cc*3] +\
                  src7[x*8 + c + cc*4] + src7[x*8 + c + cc*5] + src7[x*8 + c + cc*6] + src7[x*8 + c + cc*7] +\
                  src8[x*8 + c] + src8[x*8 + c + cc] + src8[x*8 + c + cc*2] + src8[x*8 + c + cc*3] +\
                  src8[x*8 + c + cc*4] + src8[x*8 + c + cc*5] + src8[x*8 + c + cc*6] + src8[x*8 + c + cc*7]) >> 6

  #define SCALE_CASE(defPIXEL_LOOP, defCHANEL_COMPUTION) \
    switch (cc) \
    { \
      case 1: \
        defPIXEL_LOOP( \
          defCHANEL_COMPUTION(0, cc); \
        ) \
        break; \
      case 3: \
        defPIXEL_LOOP( \
          defCHANEL_COMPUTION(0, cc); \
          defCHANEL_COMPUTION(1, cc); \
          defCHANEL_COMPUTION(2, cc); \
        ) \
        break; \
      case 4: \
        defPIXEL_LOOP( \
          defCHANEL_COMPUTION(0, cc); \
          defCHANEL_COMPUTION(1, cc); \
          defCHANEL_COMPUTION(2, cc); \
          defCHANEL_COMPUTION(3, cc); \
        ) \
        break; \
    }

  switch(scale)
  {
    case 2: SCALE_CASE(PIXEL_LOOP_2X, CHANEL_COMPUTION_2X) break;
    case 3: SCALE_CASE(PIXEL_LOOP_3X, CHANEL_COMPUTION_3X) break;
    case 4: SCALE_CASE(PIXEL_LOOP_4X, CHANEL_COMPUTION_4X) break;
    case 6: SCALE_CASE(PIXEL_LOOP_6X, CHANEL_COMPUTION_6X) break;
    case 8: SCALE_CASE(PIXEL_LOOP_8X, CHANEL_COMPUTION_8X) break;
  }

  #undef SCALE_CASE
  #undef PIXEL_LOOP_2X
  #undef PIXEL_LOOP_3X
  #undef PIXEL_LOOP_4X
  #undef CHANEL_COMPUTION_2X
  #undef CHANEL_COMPUTION_3X
  #undef CHANEL_COMPUTION_4X
  return res;
}


int main(int argc, char **argv)
{
  Bitmap *bmp;
  Bitmap *res;
  float   time;
  clock_t start;
  int     i, scale, times = 50;
  char    buf[256];

  if (argc < 3)
  {
    printf("usage: %s file.raw <scale> <runs>\n", argv[0]);
    return 0;
  }

  scale = atoi(argv[2]);
  if (argc > 3)
  {
    times = atoi(argv[3]);
  }

  bmp = load_bitmap(argv[1]);

  start = clock();
  for (i = 0; i < times; ++i)
  {
    res = scale_bitmap(bmp, scale);
    free_bitmap(res);
  }
  time = (float)(clock() - start) / CLOCKS_PER_SEC;

  res = scale_bitmap(bmp, scale);
  snprintf(buf, sizeof buf, "%s.scaled.bmp", argv[1]);
  save_bitmap(buf, res);

  free_bitmap(bmp);
  free_bitmap(res);
  printf("%d times completed in %f sec\n", times, time);

  return 0;
}
