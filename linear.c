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
