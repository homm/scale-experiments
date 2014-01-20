#include <time.h>
#include <math.h>
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
  size_t   x, y;

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
        size_t srcx = x * 4;\
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
        size_t srcx = x * 6;\
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
        size_t srcx = x * 8;\
        BODY\
      }\
    }

  #define CHANEL_COMPUTION_2X(c) \
    dst[x + c] = (src1[x*2 + c] + src1[x*2 + c + cc] + \
                  src2[x*2 + c] + src2[x*2 + c + cc]) >> 2

  #define CHANEL_COMPUTION_3X(c) \
    dst[x + c] = (src1[x*3 + c] + src1[x*3 + c + cc] + src1[x*3 + c + cc*2] +\
                  src2[x*3 + c] + src2[x*3 + c + cc] + src2[x*3 + c + cc*2] +\
                  src3[x*3 + c] + src3[x*3 + c + cc] + src3[x*3 + c + cc*2]) / 9

  #define SRC_COMPUTION_4X(c, src) \
    src[srcx + c] + src[srcx + c + cc] + src[srcx + c + cc*2] + src[srcx + c + cc*3]
  #define CHANEL_COMPUTION_4X(c) \
    dst[x + c] = (SRC_COMPUTION_4X(c, src1) + SRC_COMPUTION_4X(c, src2) +\
                  SRC_COMPUTION_4X(c, src3) + SRC_COMPUTION_4X(c, src4)) >> 4

  #define SRC_COMPUTION_6X(c, src) \
    src[srcx + c] + src[srcx + c + cc] + src[srcx + c + cc*2] +\
    src[srcx + c + cc*3] + src[srcx + c + cc*4] + src[srcx + c + cc*5]
  #define CHANEL_COMPUTION_6X(c) \
    dst[x + c] = (SRC_COMPUTION_6X(c, src1) + SRC_COMPUTION_6X(c, src2) +\
                  SRC_COMPUTION_6X(c, src3) + SRC_COMPUTION_6X(c, src4) +\
                  SRC_COMPUTION_6X(c, src5) + SRC_COMPUTION_6X(c, src6)) / 36

  #define SRC_COMPUTION_8X(c, src) \
    src[srcx + c] + src[srcx + c + cc] + src[srcx + c + cc*2] + src[srcx + c + cc*3] +\
    src[srcx + c + cc*4] + src[srcx + c + cc*5] + src[srcx + c + cc*6] + src[srcx + c + cc*7]
  #define CHANEL_COMPUTION_8X(c) \
    dst[x + c] = (SRC_COMPUTION_8X(c, src1) + SRC_COMPUTION_8X(c, src2) +\
                  SRC_COMPUTION_8X(c, src3) + SRC_COMPUTION_8X(c, src4) +\
                  SRC_COMPUTION_8X(c, src5) + SRC_COMPUTION_8X(c, src6) +\
                  SRC_COMPUTION_8X(c, src7) + SRC_COMPUTION_8X(c, src8)) >> 6

  #define SCALE_CASE(defPIXEL_LOOP, defCHANEL_COMPUTION) \
    switch (cc) \
    { \
      case 1: \
        defPIXEL_LOOP( \
          defCHANEL_COMPUTION(0); \
        ) \
        break; \
      case 3: \
        defPIXEL_LOOP( \
          defCHANEL_COMPUTION(0); \
          defCHANEL_COMPUTION(1); \
          defCHANEL_COMPUTION(2); \
        ) \
        break; \
      case 4: \
        defPIXEL_LOOP( \
          defCHANEL_COMPUTION(0); \
          defCHANEL_COMPUTION(1); \
          defCHANEL_COMPUTION(2); \
          defCHANEL_COMPUTION(3); \
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

  return res;
  #undef SCALE_CASE
  #undef PIXEL_LOOP_2X
  #undef PIXEL_LOOP_3X
  #undef PIXEL_LOOP_4X
  #undef CHANEL_COMPUTION_2X
  #undef CHANEL_COMPUTION_3X
  #undef CHANEL_COMPUTION_4X
}


Bitmap *linear_resize_bitmap(Bitmap *bmp, int width)
{
  Bitmap  *res;
  uint8_t *src, *dst;
  uint16_t*psrccoof;
  uint32_t*psrcx;
  size_t   srcrow, dstrow, cc;
  size_t   x, y, xcc;

  double srcxres = (double) bmp->width / width;
  if (srcxres < 1.0 || srcxres > 2.0)
  {
    printf("image width/new width proportion should be in range 1 to 2\n");
    printf("with image width is %d and new width is %d it will be %f\n",
      bmp->width, width, srcxres);
    return NULL;
  }

  res = (Bitmap *)calloc(1, sizeof(Bitmap));
  res->width = width;
  res->height = bmp->height;
  res->channels = bmp->channels;
  res->ptr = malloc(res->width * res->height * res->channels);

  cc = bmp->channels;
  dstrow = res->width * cc;
  srcrow = bmp->width * cc;

  psrcx = malloc(sizeof(uint32_t) * width);
  psrccoof = malloc(sizeof(uint16_t) * width * 3);
  for (x = 0; x < width; x+= 1)
  {
    double srcx;
    double coof = modf(x * srcxres, &srcx);
    double srcnextx = (x + 1) * srcxres - srcx;
    psrcx[x] = ((uint32_t)srcx) * cc;
    psrccoof[x * 3] = (uint16_t) ((1.0 - coof) / srcxres * 4096.0);
    psrccoof[x * 3 + 1] = (uint16_t) (fmin(1.0, srcnextx - 1.0) / srcxres * 4096.0);
    psrccoof[x * 3 + 2] = (uint16_t) (fmax(0.0, srcnextx - 2.0) / srcxres * 4096.0);
    // printf("%d\n", psrccoof[x * 3] + psrccoof[x * 3 + 1] + psrccoof[x * 3 + 2]);
  }

  switch (cc)
  {
    case 3:
      for (y = 0; y < res->height; y++)
      {
        src = &bmp->ptr[y * srcrow];
        dst = &res->ptr[y * dstrow];
        for (x = 0, xcc = 0; x < width; x += 1, xcc += cc)
        {
          uint32_t srcx = psrcx[x];
          uint16_t coof1 = psrccoof[xcc];
          uint16_t coof2 = psrccoof[xcc + 1];
          uint16_t coof3 = psrccoof[xcc + 2];
          dst[xcc + 0] = (src[srcx + 0] * coof1 +\
                          src[srcx + 0 + cc] * coof2 +
                          src[srcx + 0 + cc*2] * coof3) >> 12;
          dst[xcc + 1] = (src[srcx + 1] * coof1 +
                          src[srcx + 1 + cc] * coof2 +
                          src[srcx + 1 + cc*2] * coof3) >> 12;
          dst[xcc + 2] = (src[srcx + 2] * coof1 +
                          src[srcx + 2 + cc] * coof2 +
                          src[srcx + 2 + cc*2] * coof3) >> 12;
        }
      }
      break;
  }

  free(psrcx);
  free(psrccoof);
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
    res = linear_resize_bitmap(bmp, scale);
    if ( ! res)
    {
      free_bitmap(bmp);
      return 0;
    }
    free_bitmap(res);
  }
  time = (float)(clock() - start) / CLOCKS_PER_SEC;

  res = linear_resize_bitmap(bmp, scale);
  snprintf(buf, sizeof buf, "%s.scaled.bmp", argv[1]);
  save_bitmap(buf, res);

  free_bitmap(bmp);
  free_bitmap(res);
  printf("%d times completed in %f sec\n", times, time);

  return 0;
}
