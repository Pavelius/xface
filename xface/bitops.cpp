#include "crt.h"
#include "bitops.h"

#ifndef __GNUC__
#pragma optimize("t", on)
#endif

void draw::bop::set8(unsigned char* d, int d_scan, int width, int height, unsigned char c1)
{
	while(height-- > 0)
	{
		int i = width;
		unsigned char* d1 = d;
		while(i-- > 0)
			*d1++ = c1;
		d += d_scan;
	}
}

void draw::bop::set32(unsigned char* d, int d_scan, int width, int height, color c1)
{
	while(height-- > 0)
	{
		color* d1 = (color*)d;
		color* d2 = d1 + width;
		while(d1 < d2)
			*d1++ = c1;
		d += d_scan;
	}
}

void draw::bop::set32(unsigned char* d, int d_scan, int width, int height, color c1, unsigned char alpha)
{
	if(alpha == 0)
		return;
	else if(alpha >= 255)
	{
		set32(d, d_scan, width, height, c1);
		return;
	}
	while(height-- > 0)
	{
		color* d1 = (color*)d;
		color* d2 = d1 + width;
		if(alpha == 128)
		{
			while(d1 < d2)
			{
				d1->r = (d1->r + c1.r) >> 1;
				d1->g = (d1->g + c1.g) >> 1;
				d1->b = (d1->b + c1.b) >> 1;
				d1++;
			}
		}
		else
		{
			while(d1 < d2)
			{
				d1->r = (d1->r*(255 - alpha) + c1.r*alpha) >> 8;
				d1->g = (d1->g*(255 - alpha) + c1.g*alpha) >> 8;
				d1->b = (d1->b*(255 - alpha) + c1.b*alpha) >> 8;
				d1++;
			}
		}
		d += d_scan;
	}
}

void draw::bop::set8(unsigned char* d, int d_scan, int width, int height, unsigned char* decode)
{
	while(height-- > 0)
	{
		int i = width;
		unsigned char* d1 = d;
		while(i-- > 0)
		{
			*d1 = decode[*d1];
			d1++;
		}
		d += d_scan;
	}
}

void draw::bop::raw8t(unsigned char* d, int d_scan, unsigned char* s, int s_scan, int w, int h)
{
	if(h <= 0 || w <= 0)
		return;
	do
	{
		unsigned char* d2 = d;
		unsigned char* sb = s;
		unsigned char* se = s + w;
		while(sb < se)
		{
			unsigned char by = *sb++;
			if(by)
				*d2++ = by;
			else
				d2++;
		}
		s += s_scan;
		d += d_scan;
	} while(--h);
}

void draw::bop::raw8m(unsigned char* d, int d_scan, unsigned char* s, int s_scan, int width, int h)
{
	if(h <= 0 || width <= 0)
		return;
	do
	{
		unsigned char* d2 = d;
		unsigned char* sb = s;
		unsigned char* se = s + width;
		while(sb < se)
			*d2-- = *sb++;
		s += s_scan;
		d += d_scan;
	} while(--h);
}

void draw::bop::raw8tm(unsigned char* d, int d_scan, unsigned char* s, int s_scan, int width, int h)
{
	if(h <= 0 || width <= 0)
		return;
	do
	{
		unsigned char* d2 = d;
		unsigned char* sb = s;
		unsigned char* se = s + width;
		while(sb < se)
		{
			unsigned char by = *sb++;
			if(by)
				*d2-- = by;
			else
				d2--;
		}
		s += s_scan;
		d += d_scan;
	} while(--h);
}

void draw::bop::raw832(unsigned char* d, int d_scan, unsigned char* s, int s_scan, int width, int height, const color* pallette)
{
	const int cbd = 4;
	while(height-- > 0)
	{
		unsigned char* p1 = d;
		unsigned char* sb = s;
		unsigned char* se = s + width;
		while(sb < se)
		{
			*((color*)p1) = pallette[*sb++];
			p1 += cbd;
		}
		s += s_scan;
		d += d_scan;
	}
}

void draw::bop::raw832m(unsigned char* d, int d_scan, unsigned char* s, int s_scan, int width, int height, const color* pallette)
{
	const int cbd = 4;
	while(height-- > 0)
	{
		unsigned char* p1 = d;
		unsigned char* sb = s;
		unsigned char* se = s + width;
		while(sb < se)
		{
			*((color*)p1) = pallette[*sb++];
			p1 -= cbd;
		}
		s += s_scan;
		d += d_scan;
	}
}

void draw::bop::raw32(unsigned char* d, int d_scan, unsigned char* s, int s_scan, int width, int height)
{
	const int cbs = 3;
	const int cbd = 4;
	if(width <= 0)
		return;
	while(height-- > 0)
	{
		register unsigned char* sb = s;
		register unsigned char* se = s + width*cbs;
		register unsigned char* p1 = d;
		while(sb < se)
		{
			p1[0] = sb[0];
			p1[1] = sb[1];
			p1[2] = sb[2];
			sb += cbs;
			p1 += cbd;
		}
		s += s_scan;
		d += d_scan;
	}
}

void draw::bop::raw32m(unsigned char* d, int d_scan, unsigned char* s, int s_scan, int width, int height)
{
	const int cbs = 3;
	const int cbd = 4;
	if(width <= 0)
		return;
	while(height-- > 0)
	{
		register unsigned char* sb = s;
		register unsigned char* se = s + width*cbs;
		register unsigned char* p1 = d;
		while(sb < se)
		{
			p1[0] = sb[0];
			p1[1] = sb[1];
			p1[2] = sb[2];
			sb += cbs;
			p1 -= cbd;
		}
		s += s_scan;
		d += d_scan;
	}
}

// (00) end of line
// (01 - 7F) draw count of pixels
// (80, XX) draw count of XX pixels
// (81 - 9F) draw count of (B-0xC0) bytes of alpha AA pixels
// (A0, XX) skip count of XX pixels
// A1 - FF skip count of (b-0xB0) pixels
void draw::bop::rle8(unsigned char* p1, int d1, unsigned char* s, int h, const unsigned char* s1, const unsigned char* s2)
{
	const int cbs = 1;
	const int cbd = 1;
	unsigned char* d = p1;
	while(true)
	{
		unsigned char c = *s++;
		if(c == 0)
		{
			p1 += d1;
			s1 += d1;
			s2 += d1;
			if(--h == 0)
				break;
			d = p1;
		}
		else if(c <= 0x9F)
		{
			unsigned char cb;
			bool need_correct_s = false;
			// count
			if(c <= 0x7F)
			{
				need_correct_s = true;
				cb = c;
			}
			else if(c == 0x80)
				cb = *s++;
			else
				cb = c - 0x80;
			// clip left invisible part
			if(d + cb*cbd <= s1 || d > s2)
			{
				d += cb*cbd;
				if(need_correct_s)
					s += cb;
				continue;
			}
			else if(d < s1)
			{
				unsigned char sk = (s1 - d) / cbd;
				d += sk*cbd;
				if(need_correct_s)
					s += sk;
				cb -= sk;
			}
			// visible part
			do
			{
				if(d >= s2)
					break;
				*d++ = *s++;
			} while(--cb);
			// right clip part
			if(cb)
			{
				if(need_correct_s)
					s += cb;
				d += cb*cbd;
			}
		}
		else
		{
			if(c == 0xA0)
				d += (*s++)*cbd;
			else
				d += (c - 0xA0)*cbd;
		}
	}
}

bool draw::bop::rle8h(int x1, int y, int dy, int dx, unsigned char* s, int h, const point mouse)
{
	int x = x1;
	while(true)
	{
		unsigned char c = *s++;
		if(c == 0)
		{
			if(--h == 0)
				break;
			y += dy;
			x = x1;
		}
		else if(c <= 0x9F)
		{
			unsigned char cb;
			// count
			if(c <= 0x7F)
				cb = c;
			else if(c == 0x80)
				cb = *s++;
			else
				cb = c - 0x80;
			if(dx > 0)
			{
				int x2 = x + cb;
				if(mouse.y == y && mouse.x >= x && mouse.x <= x2)
					return true;
				x = x2;
			}
			else
			{
				int x2 = x - cb;
				if(mouse.y == y && mouse.x >= x2 && mouse.x <= x)
					return true;
				x = x2;
			}
			s += cb;
		}
		else
		{
			if(c == 0xA0)
				c = (*s++);
			else
				c = (c - 0xA0);
			if(dx > 0)
				x += c;
			else
				x -= c;
		}
	}
	return false;
}

void draw::bop::rle8m(unsigned char* p1, int d1, unsigned char* s, int h, const unsigned char* s1, const unsigned char* s2)
{
	const int	cbs = 1;
	const int	cbd = 1;
	unsigned char* d = p1;
	while(true)
	{
		unsigned char c = *s++;
		if(c == 0)
		{
			p1 += d1;
			s1 += d1;
			s2 += d1;
			d = p1;
			if(--h == 0)
				break;
		}
		else if(c <= 0x9F)
		{
			unsigned char cb;
			// count
			if(c <= 0x7F)
				cb = c;
			else if(c == 0x80)
				cb = *s++;
			else
				cb = c - 0x80;
			// clip left invisible part
			if(d - (cb*cbd) >= s2 || d < s1)
			{
				s += cb*cbs;
				d -= cb*cbd;
				continue;
			}
			else if(d >= s2)
			{
				unsigned char sk = 1 + (d - s2) / cbd;
				d -= sk*cbd;
				s += sk*cbs;
				cb -= sk;
				if(!cb)
					continue;
			}
			// visible part
			// no alpha or modification
			do
			{
				if(d < s1)
					break;
				d[0] = s[0];
				s += cbs;
				d -= cbd;
			} while(--cb);
			// right clip part
			if(cb)
			{
				d -= cb*cbd;
				s += cb*cbs;
			}
		}
		else
		{
			if(c == 0xA0)
				d -= (*s++)*cbd;
			else
				d -= (c - 0xA0)*cbd;
		}
	}
}

// (00) end of line
// (01 - 7F) draw count of pixels
// (80, XX, AA) draw count of XX bytes of alpha AA pixels
// (81 - 9F, AA) draw count of (B-0xC0) bytes of alpha AA pixels
// (A0, XX) skip count of XX pixels
// A1 - FF skip count of (b-0xB0) pixels
// each pixel has b,g,r value
void draw::bop::rle32(unsigned char* p1, int d1, unsigned char* s, int h, const unsigned char* s1, const unsigned char* s2, unsigned char alpha)
{
	const int cbs = 3;
	const int cbd = 32 / 8;
	unsigned char* d = p1;
	if(!alpha)
		return;
	while(true)
	{
		unsigned char c = *s++;
		if(c == 0)
		{
			p1 += d1;
			s1 += d1;
			s2 += d1;
			d = p1;
			if(--h == 0)
				break;
		}
		else if(c <= 0x9F)
		{
			unsigned char ap, cb;
			// count
			if(c <= 0x7F)
			{
				cb = c;
				ap = 0xFF;
			}
			else if(c == 0x80)
			{
				cb = *s++;
				ap = *s++;
			}
			else
			{
				cb = c - 0x80;
				ap = *s++;
			}
			// clip left invisible part
			if(d + cb*cbd <= s1 || d > s2)
			{
				d += cb*cbd;
				s += cb*cbs;
				continue;
			}
			else if(d < s1)
			{
				unsigned char sk = (s1 - d) / cbd;
				d += sk*cbd;
				s += sk*cbs;
				cb -= sk;
			}
			// visible part
			if(ap == 0xFF && alpha == 0xFF)
			{
				do
				{
					if(d >= s2)
						break;
					d[0] = s[0];
					d[1] = s[1];
					d[2] = s[2];
					s += cbs;
					d += cbd;
				} while(--cb);
			}
			else
			{
				ap = (ap*alpha) / 256;
				do
				{
					if(d >= s2)
						break;
					d[0] = (((int)d[0] * (255 - ap)) + ((s[0])*(ap))) >> 8;
					d[1] = (((int)d[1] * (255 - ap)) + ((s[1])*(ap))) >> 8;
					d[2] = (((int)d[2] * (255 - ap)) + ((s[2])*(ap))) >> 8;
					s += cbs;
					d += cbd;
				} while(--cb);
			}
			// right clip part
			if(cb)
			{
				s += cb*cbs;
				d += cb*cbd;
			}
		}
		else
		{
			if(c == 0xA0)
				d += (*s++)*cbd;
			else
				d += (c - 0xA0)*cbd;
		}
	}
}

void draw::bop::rle32m(unsigned char* p1, int d1, unsigned char* s, int h, const unsigned char* s1, const unsigned char* s2, unsigned char alpha)
{
	const int cbs = 3;
	const int cbd = 32 / 8;
	unsigned char* d = p1;
	if(!alpha)
		return;
	while(true)
	{
		unsigned char c = *s++;
		if(c == 0)
		{
			p1 += d1;
			s1 += d1;
			s2 += d1;
			d = p1;
			if(--h == 0)
				break;
		}
		else if(c <= 0x9F)
		{
			unsigned char ap, cb;
			// count
			if(c <= 0x7F)
			{
				cb = c;
				ap = 0xFF;
			}
			else if(c == 0x80)
			{
				cb = *s++;
				ap = *s++;
			}
			else
			{
				cb = c - 0x80;
				ap = *s++;
			}
			// clip left invisible part
			if(d - (cb*cbd) >= s2 || d < s1)
			{
				s += cb*cbs;
				d -= cb*cbd;
				continue;
			}
			else if(d >= s2)
			{
				unsigned char sk = 1 + (d - s2) / cbd;
				d -= sk*cbd;
				s += sk*cbs;
				cb -= sk;
				if(!cb)
					continue;
			}
			// visible part
			if(ap == 0xFF && alpha == 0xFF)
			{
				// no alpha or modification
				do
				{
					if(d < s1)
						break;
					d[0] = s[0];
					d[1] = s[1];
					d[2] = s[2];
					s += cbs;
					d -= cbd;
				} while(--cb);
			}
			else
			{
				// alpha channel
				ap = (ap*alpha) / 256;
				do
				{
					if(d < s1)
						break;
					d[0] = (((int)d[0] * (255 - ap)) + ((s[0])*(ap))) >> 8;
					d[1] = (((int)d[1] * (255 - ap)) + ((s[1])*(ap))) >> 8;
					d[2] = (((int)d[2] * (255 - ap)) + ((s[2])*(ap))) >> 8;
					d -= cbd;
					s += cbs;
				} while(--cb);
			}
			// right clip part
			if(cb)
			{
				d -= cb*cbd;
				s += cb*cbs;
			}
		}
		else
		{
			if(c == 0xA0)
				d -= (*s++)*cbd;
			else
				d -= (c - 0xA0)*cbd;
		}
	}
}

void draw::bop::rle832(unsigned char* p1, int d1, unsigned char* s, int h, const unsigned char* s1, const unsigned char* s2, unsigned char alpha, const color* pallette)
{
	const int cbs = 3;
	const int cbd = 32 / 8;
	unsigned char* d = p1;
	if(!alpha)
		return;
	while(true)
	{
		unsigned char c = *s++;
		if(c == 0)
		{
			p1 += d1;
			s1 += d1;
			s2 += d1;
			if(--h == 0)
				break;
			d = p1;
		}
		else if(c <= 0x9F)
		{
			unsigned char ap = alpha, cb;
			bool need_correct_s = false;
			// count
			if(c <= 0x7F)
			{
				need_correct_s = true;
				cb = c;
			}
			else if(c == 0x80)
			{
				cb = *s++;
				ap >>= 1;
			}
			else
			{
				cb = c - 0x80;
				ap >>= 1;
			}
			// clip left invisible part
			if(d + cb*cbd <= s1 || d > s2)
			{
				d += cb*cbd;
				if(need_correct_s)
					s += cb;
				continue;
			}
			else if(d < s1)
			{
				unsigned char sk = (s1 - d) / cbd;
				d += sk*cbd;
				if(need_correct_s)
					s += sk;
				cb -= sk;
			}
			// visible part
			if(ap == alpha)
			{
				if(ap == 0xFF)
				{
					do
					{
						if(d >= s2)
							break;
						*((color*)d) = pallette[*s++];
						d += cbd;
					} while(--cb);
				}
				else
				{
					do
					{
						if(d >= s2)
							break;
						unsigned char* s1 = (unsigned char*)&pallette[*s++];
						d[0] = (((int)d[0] * (255 - ap)) + ((s1[0])*(ap))) >> 8;
						d[1] = (((int)d[1] * (255 - ap)) + ((s1[1])*(ap))) >> 8;
						d[2] = (((int)d[2] * (255 - ap)) + ((s1[2])*(ap))) >> 8;
						d += cbd;
					} while(--cb);
				}
			}
			else if(ap == 0x7F)
			{
				do
				{
					if(d >= s2)
						break;
					d[0] >>= 1;
					d[1] >>= 1;
					d[2] >>= 1;
					d += cbd;
				} while(--cb);
			}
			else
			{
				ap = 255 - ap;
				do
				{
					if(d >= s2)
						break;
					d[0] = (((int)d[0] * ap)) >> 8;
					d[1] = (((int)d[1] * ap)) >> 8;
					d[2] = (((int)d[2] * ap)) >> 8;
					d += cbd;
				} while(--cb);
			}
			// right clip part
			if(cb)
			{
				if(need_correct_s)
					s += cb;
				d += cb*cbd;
			}
		}
		else
		{
			if(c == 0xA0)
				d += (*s++)*cbd;
			else
				d += (c - 0xA0)*cbd;
		}
	}
}

void draw::bop::rle832m(unsigned char* p1, int d1, unsigned char* s, int h, const unsigned char* s1, const unsigned char* s2, unsigned char alpha, const color* pallette)
{
	const int cbs = 3;
	const int cbd = 32 / 8;
	unsigned char* d = p1;
	if(!alpha)
		return;
	while(true)
	{
		unsigned char c = *s++;
		if(c == 0)
		{
			p1 += d1;
			s1 += d1;
			s2 += d1;
			d = p1;
			if(--h == 0)
				break;
		}
		else if(c <= 0x9F)
		{
			unsigned char ap = alpha, cb;
			bool need_correct_s = false;
			// count
			if(c <= 0x7F)
			{
				need_correct_s = true;
				cb = c;
			}
			else if(c == 0x80)
			{
				cb = *s++;
				ap >>= 1;
			}
			else
			{
				cb = c - 0x80;
				ap >>= 1;
			}
			// clip left invisible part
			if(d - (cb*cbd) >= s2 || d < s1)
			{
				d -= cb*cbd;
				if(need_correct_s)
					s += cb;
				continue;
			}
			else if(d >= s2)
			{
				unsigned char sk = (d - s2) / cbd;
				d -= sk*cbd;
				if(need_correct_s)
					s += sk;
				cb -= sk;
			}
			// visible part
			if(ap == alpha)
			{
				if(ap == 0xFF)
				{
					do
					{
						if(d < s1)
							break;
						*((color*)d) = pallette[*s++];
						d -= cbd;
					} while(--cb);
				}
				else
				{
					do
					{
						if(d < s1)
							break;
						unsigned char* s1 = (unsigned char*)&pallette[*s++];
						d[0] = (((int)d[0] * (255 - ap)) + ((s1[0])*(ap))) >> 8;
						d[1] = (((int)d[1] * (255 - ap)) + ((s1[1])*(ap))) >> 8;
						d[2] = (((int)d[2] * (255 - ap)) + ((s1[2])*(ap))) >> 8;
						d -= cbd;
					} while(--cb);
				}
			}
			else if(ap == 0x7F)
			{
				do
				{
					if(d < s1)
						break;
					d[0] >>= 1;
					d[1] >>= 1;
					d[2] >>= 1;
					d -= cbd;
				} while(--cb);
			}
			else
			{
				ap = 255 - ap;
				do
				{
					if(d < s1)
						break;
					d[0] = (((int)d[0] * ap)) >> 8;
					d[1] = (((int)d[1] * ap)) >> 8;
					d[2] = (((int)d[2] * ap)) >> 8;
					d -= cbd;
				} while(--cb);
			}
			// right clip part
			if(cb)
			{
				if(need_correct_s)
					s += cb;
				d -= cb*cbd;
			}
		}
		else
		{
			if(c == 0xA0)
				d -= (*s++)*cbd;
			else
				d -= (c - 0xA0)*cbd;
		}
	}
}

void draw::bop::alc32(unsigned char* d, int d_scan, const unsigned char* s, int height, const unsigned char* clip_x1, const unsigned char* clip_x2, color c1, bool italic)
{
	const int cbs = 3;
	const int cbd = 4;
	unsigned char* p = d;
	while(true)
	{
		unsigned char c = *s++;
		if(c == 0)
		{
			d += d_scan;
			clip_x1 += d_scan;
			clip_x2 += d_scan;
			if(italic && (height & 1) != 0)
				d -= cbd;
			p = d;
			if(--height == 0)
				break;
		}
		else if(c <= 0x7F)
		{
			// clip left invisible part
			if(p + (c*cbd) <= clip_x1 || p > clip_x2)
			{
				p += c*cbd;
				s += c*cbs;
				continue;
			}
			else if(p < clip_x1)
			{
				unsigned char sk = (clip_x1 - p) / cbd;
				p += sk*cbd;
				s += sk*cbs;
				c -= sk;
			}
			// visible part
			do
			{
				if(p >= clip_x2)
					break;
				p[0] = ((p[0] * (255 - s[0])) + (c1.b*(s[0]))) >> 8;
				p[1] = ((p[1] * (255 - s[1])) + (c1.g*(s[1]))) >> 8;
				p[2] = ((p[2] * (255 - s[2])) + (c1.r*(s[2]))) >> 8;
				p += cbd;
				s += cbs;
			} while(--c);
			// right clip part
			if(c)
			{
				p += c*cbd;
				s += c*cbs;
			}
		}
		else
		{
			if(c == 0x80)
				p += (*s++)*cbd;
			else
				p += (c - 0x80)*cbd;
		}
	}
}

static void scale_line_32(unsigned char* dst, unsigned char* src, int sw, int tw)
{
	const int cbd = 4;
	int NumPixels = tw;
	int IntPart = (sw / tw)*cbd;
	int FractPart = sw % tw;
	int E = 0;
	while(NumPixels-- > 0)
	{
		*((unsigned*)dst) = *((unsigned*)src);
		dst += cbd;
		src += IntPart;
		E += FractPart;
		if(E >= tw)
		{
			E -= tw;
			src += cbd;
		}
	}
}

void draw::bop::scale32(
	unsigned char* d, int d_scan, int d_width, int d_height,
	unsigned char* s, int s_scan, int s_width, int s_height)
{
	if(!d_width || !d_height || !s_width || !s_height)
		return;
	const int cbd = 4;
	int NumPixels = d_height;
	int IntPart = (s_height / d_height) * s_scan;
	int FractPart = s_height % d_height;
	int E = 0;
	unsigned char* PrevSource = 0;
	while(NumPixels-- > 0)
	{
		if(s == PrevSource)
			memcpy(d, d - d_scan, d_width*cbd);
		else
		{
			scale_line_32(d, s, s_width, d_width);
			PrevSource = s;
		}
		d += d_scan;
		s += IntPart;
		E += FractPart;
		if(E >= d_height)
		{
			E -= d_height;
			s += s_scan;
		}
	}
}

static void scale_line_8(unsigned char* dst, unsigned char* src, int sw, int tw)
{
	const int cbd = 1;
	int NumPixels = tw;
	int IntPart = (sw / tw)*cbd;
	int FractPart = sw % tw;
	int E = 0;
	while(NumPixels-- > 0)
	{
		*((unsigned char*)dst) = *((unsigned char*)src);
		dst += cbd;
		src += IntPart;
		E += FractPart;
		if(E >= tw)
		{
			E -= tw;
			src += cbd;
		}
	}
}

void draw::bop::scale8(
	unsigned char* d, int d_scan, int d_width, int d_height,
	unsigned char* s, int s_width, int s_height, int s_scan)
{
	const int cbd = 1;
	int NumPixels = d_height;
	int IntPart = (s_height / d_height) * s_scan;
	int FractPart = s_height % d_height;
	int E = 0;
	unsigned char* PrevSource = 0;
	while(NumPixels-- > 0)
	{
		if(s == PrevSource)
			memcpy(d, d - d_scan, d_width*cbd);
		else
		{
			scale_line_32(d, s, s_width, d_width);
			PrevSource = s;
		}
		d += d_scan;
		s += IntPart;
		E += FractPart;
		if(E >= d_height)
		{
			E -= d_height;
			s += s_scan;
		}
	}
}

void draw::bop::cpy(unsigned char* d, int d_scan, unsigned char* s, int s_scan, int width, int height, int bytes_per_pixel)
{
	if(height <= 0 || width <= 0)
		return;
	int width_bytes = width*bytes_per_pixel;
	do
	{
		memcpy(d, s, width_bytes);
		s += s_scan;
		d += d_scan;
	} while(--height);
}

void draw::bop::cpy32t(unsigned char* d, int d_scan, unsigned char* s, int s_scan, int width, int height)
{
	if(height <= 0 || width <= 0)
		return;
	do {
		color* d2 = (color*)d;
		color* sb = (color*)s;
		color* se = sb + width;
		while(sb < se) {
			if(!sb->a) {
				d2++;
				sb++;
			} else if(sb->a == 0xFF)
				*d2++ = *sb++;
			else {
				auto ap = sb->a;
				d2->r = (((int)d2->r * (255 - ap)) + ((sb->r)*(ap))) >> 8;
				d2->g = (((int)d2->g * (255 - ap)) + ((sb->g)*(ap))) >> 8;
				d2->b = (((int)d2->b * (255 - ap)) + ((sb->b)*(ap))) >> 8;
				d2++; sb++;
			}
		}
		s += s_scan;
		d += d_scan;
	} while(--height);
}