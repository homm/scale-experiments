This is me experiments about images scaling performance.
There is simple 24bit `.bmp` decoder/encoder and number of
scale algorithms.

My current achievements:
* scale.c/scale_bitmap — upltafast (about 1100 megapixels/s) scaling to
  fixed ratio (now it is 2x, 3x, 4x, 6x, 8x). The extra pixels are cut off.
* linear.c/linear_resize_bitmap — superfast (about 195 megapixels/s) scaling
  to ratio from 1x to 2x.
* inverse.c/inverse_resize_bitmap — fast (about 81 megapixels/s) scaling to
  ratio from 1x. Strict algorithm which operates from a position of source image.
* inverse.c/fast_inverse_resize_bitmap — superfast (about 400 megapixels/s)
  scaling to ratio from 3x. Works similar to previos, but not so strict and
  therefore inaccurate with ration from 1x to ≈3x.

For comparison:
* GraphicsMagick resize algorithm with Sinc filter: from 12 to 24 megapixels/s.
* GraphicsMagick resize algorithm with Triangle filter: from 22 to 62 megapixels/s.
* Python's PIL resize with ANTIALIAS filter (Lanczos in fact): from 11 to 22 megapixels/s.

All test have been done on Intel Core i5 4258U CPU with 
6000×3068 pixels source image.

The best way to launch:

    gcc main.c -lm -o scale -O2 && time ./scale louvre.bmp louvre.scaled.bmp <params>

Room for optimization: handle pixel at once with SSE2.
