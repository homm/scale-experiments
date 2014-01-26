Bitmap inverse_resize_bitmap(Bitmap bmp, size_t width, size_t height)
{
  Bitmap   res;
  uint8_t *src, *dst;
  uint32_t*buf1, *buf2, *buf3;
  uint16_t*pdstxcoof;
  uint32_t*pdstx;
  float   *pcoofsums;
  size_t   srcrow, dstrow, cc;
  size_t   x, y, xcc;
  size_t   lastrow = 0;
  double (*easing)(double) = square_easing;

  size_t bmp_width = bmp->width;
  size_t bmp_height = bmp->height;
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
    double coof1f = (fmin(1.0, fract + res) - fract) / res;\
    double coof2f = easing(1.0 - coof1f) * res;\
    coof1f = easing(coof1f) * res;

  // Precompute coofs for x
  pcoofsums = calloc((width + 1), sizeof(float));
  pdstx = calloc(bmp->width, sizeof(uint32_t));
  pdstxcoof = calloc(bmp->width * 2, sizeof(uint16_t));
  for (x = 0; x < bmp_width; x+= 1)
  {
    COOF_COMPUTION(x, xres);
    pcoofsums[(size_t)dst_coord + 0] += coof1f;
    pcoofsums[(size_t)dst_coord + 1] += coof2f;
  }
  for (x = 0; x < bmp_width; x+= 1)
  {
    COOF_COMPUTION(x, xres);
    pdstxcoof[x*2 + 0] = coof1f / pcoofsums[(size_t)dst_coord + 0] * 4096.0 + .5;
    pdstxcoof[x*2 + 1] = coof2f / pcoofsums[(size_t)dst_coord + 1] * 4096.0 + .5;
    pdstx[x] = dst_coord * cc;
  }
  free(pcoofsums);

  // Precompute coofs for y
  pcoofsums = calloc((height + 1), sizeof(float));
  for (y = 0; y < bmp_height; y+= 1)
  {
    COOF_COMPUTION(y, yres);
    pcoofsums[(size_t)dst_coord + 0] += coof1f;
    pcoofsums[(size_t)dst_coord + 1] += coof2f;
  }

  #define CHANEL_COMPUTION_1X(c) \
    buf1[dstx + c     ] += src[xcc + c] * coof1;\
    buf1[dstx + c + cc] += src[xcc + c] * coof2;

  #define CHANEL_COMPUTION_2X(c) \
    buf1[dstx + c     ] += src[xcc + c] * coof11;\
    buf1[dstx + c + cc] += src[xcc + c] * coof12;\
    buf2[dstx + c     ] += src[xcc + c] * coof21;\
    buf2[dstx + c + cc] += src[xcc + c] * coof22;

  #define COMMIT() \
    dst = &res->ptr[lastrow * dstrow];\
    for (x = 0; x < dstrow; x += cc)\
    {\
      dst[x + 0] = buf1[x + 0] + 4096 * 4096 / 2 >> 24;\
      dst[x + 1] = buf1[x + 1] + 4096 * 4096 / 2 >> 24;\
      dst[x + 2] = buf1[x + 2] + 4096 * 4096 / 2 >> 24;\
    }

  switch (cc)
  {
    case 3:
      for (y = 0; y < bmp_height; y++)
      {
        COOF_COMPUTION(y, yres);
        uint16_t ycoof1 = coof1f / pcoofsums[(size_t)dst_coord + 0] * 4096.0 + .5;
        uint16_t ycoof2 = coof2f / pcoofsums[(size_t)dst_coord + 1] * 4096.0 + .5;

        if ((size_t) dst_coord > lastrow)
        {
          COMMIT();
          memset(buf1, 0, sizeof(uint32_t) * (width + 1) * cc);
          buf3 = buf1; buf1 = buf2; buf2 = buf3;
        }
        lastrow = dst_coord;

        src = &bmp->ptr[y * srcrow];
        if (ycoof2 == 0)
        {
          for (x = 0, xcc = 0; x < bmp_width; x += 1, xcc += cc)
          {
            uint32_t dstx = pdstx[x];
            uint32_t coof1 = ycoof1 * pdstxcoof[x*2 + 0];
            uint32_t coof2 = ycoof1 * pdstxcoof[x*2 + 1];

            CHANEL_COMPUTION_1X(0);
            CHANEL_COMPUTION_1X(1);
            CHANEL_COMPUTION_1X(2);
          }
        }
        else
        {
          for (x = 0, xcc = 0; x < bmp_width; x += 1, xcc += cc)
          {
            uint32_t dstx = pdstx[x];
            uint16_t xcoof1 = pdstxcoof[x*2 + 0];
            uint16_t xcoof2 = pdstxcoof[x*2 + 1];
            uint32_t coof11 = ycoof1 * xcoof1;
            uint32_t coof12 = ycoof1 * xcoof2;
            uint32_t coof21 = ycoof2 * xcoof1;
            uint32_t coof22 = ycoof2 * xcoof2;

            CHANEL_COMPUTION_2X(0);
            CHANEL_COMPUTION_2X(1);
            CHANEL_COMPUTION_2X(2);
          }
        }
      }
      COMMIT();
      break;
  }

  free(pcoofsums);
  free(buf1);
  free(buf2);
  free(pdstx);
  free(pdstxcoof);
  return res;
  #undef COOF_COMPUTION
  #undef CHANEL_COMPUTION_1X
  #undef CHANEL_COMPUTION_2X
  #undef COMMIT
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
      dst[xcc + 0] = (buf[xcc + 0] + (count >> 1)) / count;\
      dst[xcc + 1] = (buf[xcc + 1] + (count >> 1)) / count;\
      dst[xcc + 2] = (buf[xcc + 2] + (count >> 1)) / count;\
    }

  switch (cc)
  {
    case 3:
      for (y = 0; y < bmp->height; y++)
      {
        dsty = y * yres + yres / 2;

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
