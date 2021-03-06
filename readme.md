This is me experiments about images scaling performance.
There is simple 24bit `.bmp` decoder/encoder and number of
scale algorithms.

#### Current achievements
* scale.c/scale_bitmap — ultrafast (about 1100 megapixels/s) scaling to
  fixed ratio (now it is 2x, 3x, 4x, 6x, 8x). Extra pixels are cut off.
* linear.c/linear_resize_bitmap — superfast (about 195 megapixels/s) scaling
  to ratio from 1x to 2x.
* inverse.c/inverse_resize_bitmap — fast (from 100 to 150 megapixels/s)
  scaling to ratio from 1x to 16x. Strict algorithm which operates from
  a position of source image. After 16x precision can dramatically fall.
* inverse.c/fast_inverse_resize_bitmap — superfast (about 400 megapixels/s)
  scaling to ratio from 3x. Works similar to previos, but not so strict and
  therefore inaccurate with ratio from 1x to ≈3x. Have no problems
  on extreme high ratio.

#### Other libraries
* GraphicsMagick resize with Sinc filter: from 12 to 24 megapixels/s.
* GraphicsMagick resize with Triangle filter: from 22 to 62 megapixels/s.
* Python's PIL resize with Lanczos filter: from 11 to 22 megapixels/s.
* PHP-GD imagecopyresampled function: from 13.5 to 41 megapixels/s.
* AMD Framewave supersampling resize: from 34 to 62.5 megapixels/s.

All test have been done on Intel Core i5 4258U CPU with 
6000×3068 pixels source image.

#### How to use

Due to no one of algorithms covers all range of ratios, combination should be used.

1. Two or thress pass. First, image should be scaled down with scale_bitmap
   to size greater then desired from 1x to 2x times. Second, image
   should be scaled with linear_resize_bitmap. First pass can be splitted to
   vertical and horizaontal.
   Speed: 1x—2x: 195mp/s, 2x—3x: 456mp/s, 3x—4x: 676mp/s, 4x—6x: 813mp/s,
   6x—8x: 950mp/s, 8x—16x: 1010mp/s.
2. One or two pass. fast_inverse_resize_bitmap used for relatively high ratios
   (3x+), inverse_resize_bitmap used for others. If x-scale is significant
   differ from y-scale (no one of this function can be applyed), two pass
   should be made, where fast_inverse_resize_bitmap with bigger scale
   should be the first.
   Speed: 1x—2x: 100mp/s, 2x—3x: 120mp/s, 3x—4x: 125mp/s, 4x—16x: 400mp/s.

The first way is realy draft: it has geometrical and accuracy problems, but it
2-5 times faster. Second has high quality, comparable to GM's Sinc filter, and
still has 4x and more performance then any alternatives.

#### The best way to launch

    gcc main.c -lm -o scale -O2 && time ./scale louvre.bmp louvre.scaled.bmp <params>

#### Room for optimization
With sse4.1 inverse_resize_bitmap gives from 150 to 280 mb/s.
