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
