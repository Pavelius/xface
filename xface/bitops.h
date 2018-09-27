#include "color.h"
#include "point.h"

#pragma once

namespace draw {
// Make fastest bit operations
// d - destination byte array
// s - source byte array
namespace bop {
void alc32(unsigned char* d, int d_scan, const unsigned char* s, int height, const unsigned char* clip_x1, const unsigned char* clip_x2, color c1, bool italic);
void cpy(unsigned char* d, int d_scan, unsigned char* s, int s_scan, int width, int height, int bytes_per_pixel);
void cpy32t(unsigned char* d, int d_scan, unsigned char* s, int s_scan, int width, int height);
void raw32(unsigned char* d, int d_scan, unsigned char* s, int s_scan, int height, int width);
void raw32m(unsigned char* d, int d_scan, unsigned char* s, int s_scan, int height, int width);
void raw8t(unsigned char* d, int d_scan, unsigned char* s, int s_scan, int height, int width);
void raw8m(unsigned char* d, int d_scan, unsigned char* s, int s_scan, int height, int width);
void raw8tm(unsigned char* d, int d_scan, unsigned char* s, int s_scan, int height, int width);
void raw832(unsigned char* d, int d_scan, unsigned char* s, int s_scan, int width, int height, const color* pallette);
void raw832m(unsigned char* d, int d_scan, unsigned char* s, int s_scan, int width, int height, const color* pallette);
void rle8(unsigned char* d, int d_scan, unsigned char* s, int height, const unsigned char* clip_x1, const unsigned char* clip_x2);
void rle8m(unsigned char* d, int d_scan, unsigned char* s, int height, const unsigned char* clip_x1, const unsigned char* clip_x2);
bool rle8h(int x1, int y, int dx, int dy, unsigned char* s, int h, const point mouse);
void rle32(unsigned char* d, int d_scan, unsigned char* s, int height, const unsigned char* clip_x1, const unsigned char* clip_x2, unsigned char alpha);
void rle32m(unsigned char* d, int d_scan, unsigned char* s, int height, const unsigned char* clip_x1, const unsigned char* clip_x2, unsigned char alpha);
void rle832(unsigned char* d, int d_scan, unsigned char* s, int height, const unsigned char* clip_x1, const unsigned char* clip_x2, unsigned char alpha, const color* pallette);
void rle832m(unsigned char* d, int d_scan, unsigned char* s, int height, const unsigned char* clip_x1, const unsigned char* clip_x2, unsigned char alpha, const color* pallette);
void scale32(unsigned char* d, int d_scan, int d_width, int d_height, unsigned char* s, int s_scan, int s_width, int s_height);
void scale8(unsigned char* d, int d_scan, int d_width, int d_height, unsigned char* s, int s_width, int s_height, int s_scan);
void set8(unsigned char* d, int d_scan, int width, int height, unsigned char value);
void set8(unsigned char* d, int d_scan, int width, int height, unsigned char* decode);
void set32(unsigned char* d, int d_scan, int width, int height, color c1);
void set32(unsigned char* d, int d_scan, int width, int height, color c1, unsigned char alpha);
}
}
