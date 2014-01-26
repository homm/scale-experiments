#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>


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

Bitmap scale_bitmap(Bitmap bmp, int scale)
{
  Bitmap   res;
  uint8_t *src1, *src2, *src3, *src4, *src5, *src6, *src7, *src8, *dst;
  size_t   srcrow, dstrow, cc;
  size_t   x, y;

  res = (Bitmap)calloc(1, sizeof(struct Bitmap));
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

double linear_easing(double x) { return x; }
double square_easing(double x) { return x * x; }
double cubic_easing(double x) { return x * x * x; }


Bitmap linear_resize_bitmap(Bitmap bmp, size_t width, size_t height)
{
  Bitmap   res;
  uint8_t *src1, *src2, *src3, *dst;
  uint16_t*psrcxcoof;
  uint32_t*psrcx;
  size_t   srcrow, dstrow, cc;
  size_t   x, y, xcc;
  double (*easing)(double) = square_easing;

  double srcxres = (double) bmp->width / width;
  double srcyres = (double) bmp->height / height;
  if (srcxres < 1.0 || srcxres > 2.0)
  {
    printf("width should be between %d and %d. %d given\n",
      bmp->width / 2, bmp->width, width);
    return NULL;
  }
  if (srcyres < 1.0 || srcyres > 2.0)
  {
    printf("height should be between %d and %d. %d given\n",
      bmp->height / 2, bmp->height, height);
    return NULL;
  }

  res = (Bitmap)calloc(1, sizeof(struct Bitmap));
  res->width = width;
  res->height = height;
  res->channels = bmp->channels;
  res->ptr = malloc(res->width * res->height * res->channels);

  cc = bmp->channels;
  dstrow = res->width * cc;
  srcrow = bmp->width * cc;

  #define COOF_COMPUTION(dim, res) \
    double src_coord, fract = modf(dim * res, &src_coord);\
    double src_overlap = (dim + 1) * res - src_coord;\
    double coof1f = easing(1.0 - fract);\
    double coof2f = easing(fmin(1.0, src_overlap - 1.0));\
    double coof3f = easing(fmax(0.0, src_overlap - 2.0));\
    double coofsum = coof1f + coof2f + coof3f;

  psrcx = malloc(sizeof(uint32_t) * width);
  psrcxcoof = malloc(sizeof(uint16_t) * width * 3);
  for (x = 0; x < width; x+= 1)
  {
    COOF_COMPUTION(x, srcxres);
    psrcxcoof[x*3 + 0] = coof1f / coofsum * 4096.0 + .33;
    psrcxcoof[x*3 + 1] = coof2f / coofsum * 4096.0 + .33;
    psrcxcoof[x*3 + 2] = coof3f / coofsum * 4096.0 + .33;

    psrcx[x] = src_coord * cc;
  }

  #define CHANEL_COMPUTION(c) \
    dst[xcc + c] = (\
      ycoof1 * (src1[srcx + c] * xcoof1 + src1[srcx + c + cc] * xcoof2 + src1[srcx + c + cc*2] * xcoof3) +\
      ycoof2 * (src2[srcx + c] * xcoof1 + src2[srcx + c + cc] * xcoof2 + src2[srcx + c + cc*2] * xcoof3) +\
      ycoof3 * (src3[srcx + c] * xcoof1 + src3[srcx + c + cc] * xcoof2 + src3[srcx + c + cc*2] * xcoof3) +\
      4096 * 4096 / 2) >> 24;

  switch (cc)
  {
    case 3:
      for (y = 0; y < res->height - 1; y++)
      {
        COOF_COMPUTION(y, srcyres);
        uint16_t ycoof1 = coof1f / coofsum * 4096.0 + .33;
        uint16_t ycoof2 = coof2f / coofsum * 4096.0 + .33;
        uint16_t ycoof3 = coof3f / coofsum * 4096.0 + .33;

        src1 = &bmp->ptr[((uint32_t)src_coord + 0) * srcrow];
        src2 = &bmp->ptr[((uint32_t)src_coord + 1) * srcrow];
        src3 = &bmp->ptr[((uint32_t)src_coord + 2) * srcrow];
        dst = &res->ptr[y * dstrow];
        for (x = 0, xcc = 0; x < width; x += 1, xcc += cc)
        {
          uint32_t srcx = psrcx[x];
          uint16_t xcoof1 = psrcxcoof[x*3];
          uint16_t xcoof2 = psrcxcoof[x*3 + 1];
          uint16_t xcoof3 = psrcxcoof[x*3 + 2];

          CHANEL_COMPUTION(0);
          CHANEL_COMPUTION(1);
          CHANEL_COMPUTION(2);
        }
      }
      break;
  }

  free(psrcx);
  free(psrcxcoof);
  return res;
  #undef CHANEL_COMPUTION
  #undef COOF_COMPUTION
}


Bitmap inverse_resize_bitmap(Bitmap bmp, size_t width, size_t height)
{
  Bitmap   res;
  uint8_t *src, *dst;
  uint32_t*buf1, *buf2, *buf3;
  uint16_t*pdstxcoof;
  uint32_t*pdstx;
  size_t   srcrow, dstrow, cc;
  size_t   x, y, xcc;
  size_t   lastrow = 0;

  size_t bmp_width = bmp->width;
  double xres = (double) width / bmp->width;
  double yres = (double) height / bmp->height;

  res = (Bitmap)calloc(1, sizeof(struct Bitmap));
  res->width = width;
  res->height = height;
  res->channels = bmp->channels;
  res->ptr = malloc(res->width * res->height * res->channels);

  cc = bmp->channels;
  dstrow = res->width * cc;
  srcrow = bmp->width * cc;

  buf1 = calloc((width + 1) * cc, sizeof(uint32_t));
  buf2 = calloc((width + 1) * cc, sizeof(uint32_t));

  #define COOF_COMPUTION(dim, res) \
    double dst_coord, fract = modf(dim * res, &dst_coord);\
    double coof1f = fmin(1.0, fract + res) - fract;\
    double coof2f = res - coof1f;

  pdstx = calloc(bmp->width, sizeof(uint32_t));
  pdstxcoof = calloc(bmp->width * 2, sizeof(uint16_t));
  for (x = 0; x < bmp_width; x+= 1)
  {
    COOF_COMPUTION(x, xres);
    pdstxcoof[x*2 + 0] = coof1f * 4096.0 + .5;
    pdstxcoof[x*2 + 1] = coof2f * 4096.0 + .5;
    pdstx[x] = dst_coord * cc;
  }

  #define CHANEL_COMPUTION(c) \
    buf1[dstx + c     ] += src[xcc + c] * coof11;\
    buf1[dstx + c + cc] += src[xcc + c] * coof12;\
    buf2[dstx + c     ] += src[xcc + c] * coof21;\
    buf2[dstx + c + cc] += src[xcc + c] * coof22;

  switch (cc)
  {
    case 3:
      for (y = 0; y < bmp->height; y++)
      {
        COOF_COMPUTION(y, yres);
        uint16_t ycoof1 = coof1f * 4096.0 + .5;
        uint16_t ycoof2 = coof2f * 4096.0 + .5;

        // commit
        if ((size_t) dst_coord > lastrow)
        {
          dst = &res->ptr[lastrow * dstrow];
          for (x = 0; x < dstrow; x += 1)
          {
            dst[x] = buf1[x] + 4096 * 4096 / 2 >> 24;
          }
          memset(buf1, 0, sizeof(uint32_t) * (width + 1) * cc);
          buf3 = buf1; buf1 = buf2; buf2 = buf3;
        }
        lastrow = dst_coord;

        src = &bmp->ptr[y * srcrow];
        for (x = 0, xcc = 0; x < bmp_width; x += 1, xcc += cc)
        {
          uint32_t dstx = pdstx[x];
          uint16_t xcoof1 = pdstxcoof[x*2 + 0];
          uint16_t xcoof2 = pdstxcoof[x*2 + 1];
          uint32_t coof11 = ycoof1 * xcoof1;
          uint32_t coof12 = ycoof1 * xcoof2;
          uint32_t coof21 = ycoof2 * xcoof1;
          uint32_t coof22 = ycoof2 * xcoof2;

          CHANEL_COMPUTION(0);
          CHANEL_COMPUTION(1);
          CHANEL_COMPUTION(2);
        }
      }
      dst = &res->ptr[lastrow * dstrow];
      for (x = 0; x < dstrow; x += 1)
      {
        dst[x] = buf1[x] + 4096 * 4096 / 2 >> 24;
      }
      break;
  }

  free(buf1);
  free(buf2);
  free(pdstx);
  free(pdstxcoof);
  return res;
  #undef COOF_COMPUTION
  #undef CHANEL_COMPUTION
}


Bitmap fast_inverse_resize_bitmap(Bitmap bmp, size_t width, size_t height)
{
  Bitmap   res;
  uint8_t *src, *dst;
  uint32_t*buf, *pdstx, *pcountx;
  size_t   srcrow, dstrow, cc;
  size_t   x, y, xcc;
  size_t   dsty, lastdsty = 0, county = 0;

  size_t bmp_width = bmp->width;
  double xres = (double) width / bmp->width;
  double yres = (double) height / bmp->height;

  res = (Bitmap)calloc(1, sizeof(struct Bitmap));
  res->width = width;
  res->height = height;
  res->channels = bmp->channels;
  res->ptr = malloc(res->width * res->height * res->channels);

  cc = bmp->channels;
  dstrow = res->width * cc;
  srcrow = bmp->width * cc;

  buf = calloc(width * cc, sizeof(uint32_t));

  pdstx = calloc(bmp->width, sizeof(uint32_t));
  pcountx = calloc(width, sizeof(uint32_t));
  for (x = 0; x < bmp_width; x+= 1)
  {
    size_t dstx = (uint32_t)(x * xres + xres / 2);
    pdstx[x] = dstx * cc;
    pcountx[dstx] += 1;
  }

  #define COMMIT() \
    dst = &res->ptr[lastdsty * dstrow];\
    for (x = 0, xcc = 0; x < width; x += 1, xcc += cc)\
    {\
      size_t count = county * pcountx[x];\
      dst[xcc + 0] = buf[xcc + 0] / count;\
      dst[xcc + 1] = buf[xcc + 1] / count;\
      dst[xcc + 2] = buf[xcc + 2] / count;\
    }

  switch (cc)
  {
    case 3:
      for (y = 0; y < bmp->height; y++)
      {
        dsty = y * yres + yres / 2;

        // commit
        if (dsty > lastdsty)
        {
          COMMIT();
          memset(buf, 0, sizeof(uint32_t) * width * cc);
          county = 0;
        }
        lastdsty = dsty;
        county += 1;

        src = &bmp->ptr[y * srcrow];
        for (x = 0, xcc = 0; x < bmp_width; x += 1, xcc += cc)
        {
          uint32_t *pix = &buf[pdstx[x]];
          pix[0] += src[xcc + 0];
          pix[1] += src[xcc + 1];
          pix[2] += src[xcc + 2];
        }
      }
      COMMIT();
      break;
  }

  free(buf);
  free(pdstx);
  free(pcountx);
  return res;
  #undef COOF_COMPUTION
  #undef CHANEL_COMPUTION
}



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
    res = bmp;
    bmp = inverse_resize_bitmap(bmp, width - i, height - i);
    // bmp = linear_resize_bitmap(bmp, width - i, height - i);
    free_bitmap(res);
    if ( ! bmp)
    {
      return 0;
    }
  }
  time = (float)(clock() - start) / CLOCKS_PER_SEC;

  save_bitmap(bmp, argv[2]);

  free_bitmap(bmp);
  printf("%d times completed in %f sec\n", times, time);

  return 0;
}
