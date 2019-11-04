#include "color.h"
#include "crt.h"
#include "draw.h"

#ifndef __GNUC__
#pragma optimize("t", on)
#endif

extern "C" void* malloc(unsigned size);
extern "C" void* realloc(void *ptr, unsigned size);
extern "C" void	free(void* pointer);

using namespace draw;

// Default theme colors
color				colors::active;
color				colors::button;
color				colors::focus;
color				colors::form;
color				colors::window;
color				colors::text;
color				colors::border;
color				colors::edit;
color				colors::h1;
color				colors::h2;
color				colors::h3;
color				colors::special;
color				colors::tips::text;
color				colors::tips::back;
color				colors::tabs::text;
color				colors::tabs::back;
// Color context and font context
color				draw::fore;
color				draw::fore_stroke;
const sprite*		draw::font;
float				draw::linw = 1.0;
bool				draw::mouseinput = true;
color*				draw::palt;
rect				draw::clipping;
char				draw::link[4096];
hoti				draw::hot;
// Hot keys and menus
bool				sys_optimize_mouse_move = true;
rect				sys_static_area;
// Locale draw variables
static draw::surface default_surface;
draw::surface*		draw::canvas = &default_surface;
static bool			line_antialiasing = true;
// Drag
static const void*	drag_object;
point				draw::dragmouse;
// Metrics
rect				metrics::edit = {4, 4, -4, -4};
sprite*				metrics::font = (sprite*)loadb("art/fonts/font.pma");
sprite*				metrics::h1 = (sprite*)loadb("art/fonts/h1.pma");
sprite*				metrics::h2 = (sprite*)loadb("art/fonts/h2.pma");
sprite*				metrics::h3 = (sprite*)loadb("art/fonts/h3.pma");
int					metrics::scroll = 16;
int					metrics::padding = 4;

float sqrt(const float x) {
	const float xhalf = 0.5f*x;
	// get bits for floating value
	union {
		float x;
		int i;
	} u;
	u.x = x;
	u.i = 0x5f3759df - (u.i >> 1);  // gives initial guess y0
	return x * u.x*(1.5f - xhalf * u.x*u.x);// Newton step, repeating increases accuracy 
}

int isqrt(int num) {
	int res = 0;
	int bit = 1 << 30;
	// "bit" starts at the highest power of four <= the argument.
	while(bit > num)
		bit >>= 2;
	while(bit != 0) {
		if(num >= res + bit) {
			num -= res + bit;
			res = (res >> 1) + bit;
		} else
			res >>= 1;
		bit >>= 2;
	}
	return res;
}

int distance(point p1, point p2) {
	auto dx = p1.x - p2.x;
	auto dy = p1.y - p2.y;
	return isqrt(dx*dx + dy * dy);
}

static void correct(int& x1, int& y1, int& x2, int& y2) {
	if(x1 > x2)
		iswap(x1, x2);
	if(y1 > y2)
		iswap(y1, y2);
}

static bool correct(int& x1, int& y1, int& x2, int& y2, const rect& clip, bool include_edge = true) {
	correct(x1, y1, x2, y2);
	if(x2 < clip.x1 || x1 >= clip.x2 || y2 < clip.y1 || y1 >= clip.y2)
		return false;
	if(x1 < clip.x1)
		x1 = clip.x1;
	if(y1 < clip.y1)
		y1 = clip.y1;
	if(include_edge) {
		if(x2 > clip.x2)
			x2 = clip.x2;
		if(y2 > clip.y2)
			y2 = clip.y2;
	} else {
		if(x2 >= clip.x2)
			x2 = clip.x2 - 1;
		if(y2 >= clip.y2)
			y2 = clip.y2 - 1;
	}
	return true;
}

char* key2str(char* result, int key) {
	result[0] = 0;
	if(key&Ctrl)
		zcat(result, "Ctrl+");
	if(key&Alt)
		zcat(result, "Alt+");
	if(key&Shift)
		zcat(result, "Shift+");
	key = key & 0xFFFF;
	switch(key) {
	case KeyDown: zcat(result, "Down"); break;
	case KeyDelete: zcat(result, "Del"); break;
	case KeyEnd: zcat(result, "End"); break;
	case KeyEnter: zcat(result, "Enter"); break;
	case KeyHome: zcat(result, "Home"); break;
	case KeyLeft: zcat(result, "Left"); break;
	case KeyPageDown: zcat(result, "Page Down"); break;
	case KeyPageUp: zcat(result, "Page Up"); break;
	case KeyRight: zcat(result, "Right"); break;
	case KeyUp: zcat(result, "Up"); break;
	case F1: zcat(result, "F1"); break;
	case F2: zcat(result, "F2"); break;
	case F3: zcat(result, "F3"); break;
	case F4: zcat(result, "F4"); break;
	case F5: zcat(result, "F5"); break;
	case F6: zcat(result, "F6"); break;
	case F7: zcat(result, "F7"); break;
	case F8: zcat(result, "F8"); break;
	case F9: zcat(result, "F9"); break;
	case F10: zcat(result, "F10"); break;
	case F11: zcat(result, "F11"); break;
	case F12: zcat(result, "F12"); break;
	case KeySpace: zcat(result, "Space"); break;
	default:
		zcat(result, char(szupper(key - Alpha)));
		break;
	}
	return result;
}

static void set32(color* p, unsigned count) {
	auto p2 = p + count;
	while(p < p2)
		*p++ = fore;
}

static void set32a(color* p, unsigned count) {
	auto p2 = p + count;
	if(fore.a==255)
		set32(p, count);
	else if(!fore.a)
		return;
	else if(fore.a == 128) {
		while(p < p2) {
			p->r = (p->r + fore.r) >> 1;
			p->g = (p->g + fore.g) >> 1;
			p->b = (p->b + fore.b) >> 1;
			p++;
		}
	} else {
		while(p < p2) {
			p->r = (p->r*(255 - fore.a) + fore.r*fore.a) >> 8;
			p->g = (p->g*(255 - fore.a) + fore.g*fore.a) >> 8;
			p->b = (p->b*(255 - fore.a) + fore.b*fore.a) >> 8;
			p++;
		}
	}
}

static void set32(unsigned char* d, int d_scan, int width, int height, void(*proc)(color*, unsigned)) {
	while(height-- > 0) {
		proc((color*)d, width);
		d += d_scan;
	}
}

static void raw832(unsigned char* d, int d_scan, unsigned char* s, int s_scan, int width, int height, const color* pallette) {
	const int cbd = 4;
	while(height-- > 0) {
		unsigned char* p1 = d;
		unsigned char* sb = s;
		unsigned char* se = s + width;
		while(sb < se) {
			*((color*)p1) = pallette[*sb++];
			p1 += cbd;
		}
		s += s_scan;
		d += d_scan;
	}
}

static void raw832m(unsigned char* d, int d_scan, unsigned char* s, int s_scan, int width, int height, const color* pallette) {
	const int cbd = 4;
	while(height-- > 0) {
		unsigned char* p1 = d;
		unsigned char* sb = s;
		unsigned char* se = s + width;
		while(sb < se) {
			*((color*)p1) = pallette[*sb++];
			p1 -= cbd;
		}
		s += s_scan;
		d += d_scan;
	}
}

static void raw32(unsigned char* d, int d_scan, unsigned char* s, int s_scan, int width, int height) {
	const int cbs = 3;
	const int cbd = 4;
	if(width <= 0)
		return;
	while(height-- > 0) {
		unsigned char* sb = s;
		unsigned char* se = s + width * cbs;
		unsigned char* p1 = d;
		while(sb < se) {
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

static void raw32m(unsigned char* d, int d_scan, unsigned char* s, int s_scan, int width, int height) {
	const int cbs = 3;
	const int cbd = 4;
	if(width <= 0)
		return;
	while(height-- > 0) {
		unsigned char* sb = s;
		unsigned char* se = s + width * cbs;
		unsigned char* p1 = d;
		while(sb < se) {
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
// (80, XX, AA) draw count of XX bytes of alpha AA pixels
// (81 - 9F, AA) draw count of (B-0xC0) bytes of alpha AA pixels
// (A0, XX) skip count of XX pixels
// A1 - FF skip count of (b-0xB0) pixels
// each pixel has b,g,r value
static void rle32(unsigned char* p1, int d1, unsigned char* s, int h, const unsigned char* s1, const unsigned char* s2, unsigned char alpha) {
	const int cbs = 3;
	const int cbd = 32 / 8;
	unsigned char* d = p1;
	if(!alpha)
		return;
	while(true) {
		unsigned char c = *s++;
		if(c == 0) {
			p1 += d1;
			s1 += d1;
			s2 += d1;
			d = p1;
			if(--h == 0)
				break;
		} else if(c <= 0x9F) {
			unsigned char ap, cb;
			// count
			if(c <= 0x7F) {
				cb = c;
				ap = 0xFF;
			} else if(c == 0x80) {
				cb = *s++;
				ap = *s++;
			} else {
				cb = c - 0x80;
				ap = *s++;
			}
			// clip left invisible part
			if(d + cb * cbd <= s1 || d > s2) {
				d += cb * cbd;
				s += cb * cbs;
				continue;
			} else if(d < s1) {
				unsigned char sk = (s1 - d) / cbd;
				d += sk * cbd;
				s += sk * cbs;
				cb -= sk;
			}
			// visible part
			if(ap == 0xFF && alpha == 0xFF) {
				do {
					if(d >= s2)
						break;
					d[0] = s[0];
					d[1] = s[1];
					d[2] = s[2];
					s += cbs;
					d += cbd;
				} while(--cb);
			} else {
				ap = (ap*alpha) / 256;
				do {
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
			if(cb) {
				s += cb * cbs;
				d += cb * cbd;
			}
		} else {
			if(c == 0xA0)
				d += (*s++)*cbd;
			else
				d += (c - 0xA0)*cbd;
		}
	}
}

static void rle32m(unsigned char* p1, int d1, unsigned char* s, int h, const unsigned char* s1, const unsigned char* s2, unsigned char alpha) {
	const int cbs = 3;
	const int cbd = 32 / 8;
	unsigned char* d = p1;
	if(!alpha)
		return;
	while(true) {
		unsigned char c = *s++;
		if(c == 0) {
			p1 += d1;
			s1 += d1;
			s2 += d1;
			d = p1;
			if(--h == 0)
				break;
		} else if(c <= 0x9F) {
			unsigned char ap, cb;
			// count
			if(c <= 0x7F) {
				cb = c;
				ap = 0xFF;
			} else if(c == 0x80) {
				cb = *s++;
				ap = *s++;
			} else {
				cb = c - 0x80;
				ap = *s++;
			}
			// clip left invisible part
			if(d - (cb*cbd) >= s2 || d < s1) {
				s += cb * cbs;
				d -= cb * cbd;
				continue;
			} else if(d >= s2) {
				unsigned char sk = 1 + (d - s2) / cbd;
				d -= sk * cbd;
				s += sk * cbs;
				cb -= sk;
				if(!cb)
					continue;
			}
			// visible part
			if(ap == 0xFF && alpha == 0xFF) {
				// no alpha or modification
				do {
					if(d < s1)
						break;
					d[0] = s[0];
					d[1] = s[1];
					d[2] = s[2];
					s += cbs;
					d -= cbd;
				} while(--cb);
			} else {
				// alpha channel
				ap = (ap*alpha) / 256;
				do {
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
			if(cb) {
				d -= cb * cbd;
				s += cb * cbs;
			}
		} else {
			if(c == 0xA0)
				d -= (*s++)*cbd;
			else
				d -= (c - 0xA0)*cbd;
		}
	}
}

static void rle832(unsigned char* p1, int d1, unsigned char* s, int h, const unsigned char* s1, const unsigned char* s2, unsigned char alpha, const color* pallette) {
	const int cbs = 3;
	const int cbd = 32 / 8;
	unsigned char* d = p1;
	if(!alpha)
		return;
	while(true) {
		unsigned char c = *s++;
		if(c == 0) {
			p1 += d1;
			s1 += d1;
			s2 += d1;
			if(--h == 0)
				break;
			d = p1;
		} else if(c <= 0x9F) {
			unsigned char ap = alpha, cb;
			bool need_correct_s = false;
			// count
			if(c <= 0x7F) {
				need_correct_s = true;
				cb = c;
			} else if(c == 0x80) {
				cb = *s++;
				ap >>= 1;
			} else {
				cb = c - 0x80;
				ap >>= 1;
			}
			// clip left invisible part
			if(d + cb * cbd <= s1 || d > s2) {
				d += cb * cbd;
				if(need_correct_s)
					s += cb;
				continue;
			} else if(d < s1) {
				unsigned char sk = (s1 - d) / cbd;
				d += sk * cbd;
				if(need_correct_s)
					s += sk;
				cb -= sk;
			}
			// visible part
			if(ap == alpha) {
				if(ap == 0xFF) {
					do {
						if(d >= s2)
							break;
						*((color*)d) = pallette[*s++];
						d += cbd;
					} while(--cb);
				} else {
					do {
						if(d >= s2)
							break;
						unsigned char* s1 = (unsigned char*)&pallette[*s++];
						d[0] = (((int)d[0] * (255 - ap)) + ((s1[0])*(ap))) >> 8;
						d[1] = (((int)d[1] * (255 - ap)) + ((s1[1])*(ap))) >> 8;
						d[2] = (((int)d[2] * (255 - ap)) + ((s1[2])*(ap))) >> 8;
						d += cbd;
					} while(--cb);
				}
			} else if(ap == 0x7F) {
				do {
					if(d >= s2)
						break;
					d[0] >>= 1;
					d[1] >>= 1;
					d[2] >>= 1;
					d += cbd;
				} while(--cb);
			} else {
				ap = 255 - ap;
				do {
					if(d >= s2)
						break;
					d[0] = (((int)d[0] * ap)) >> 8;
					d[1] = (((int)d[1] * ap)) >> 8;
					d[2] = (((int)d[2] * ap)) >> 8;
					d += cbd;
				} while(--cb);
			}
			// right clip part
			if(cb) {
				if(need_correct_s)
					s += cb;
				d += cb * cbd;
			}
		} else {
			if(c == 0xA0)
				d += (*s++)*cbd;
			else
				d += (c - 0xA0)*cbd;
		}
	}
}

static void rle832m(unsigned char* p1, int d1, unsigned char* s, int h, const unsigned char* s1, const unsigned char* s2, unsigned char alpha, const color* pallette) {
	const int cbs = 3;
	const int cbd = 32 / 8;
	unsigned char* d = p1;
	if(!alpha)
		return;
	while(true) {
		unsigned char c = *s++;
		if(c == 0) {
			p1 += d1;
			s1 += d1;
			s2 += d1;
			d = p1;
			if(--h == 0)
				break;
		} else if(c <= 0x9F) {
			unsigned char ap = alpha, cb;
			bool need_correct_s = false;
			// count
			if(c <= 0x7F) {
				need_correct_s = true;
				cb = c;
			} else if(c == 0x80) {
				cb = *s++;
				ap >>= 1;
			} else {
				cb = c - 0x80;
				ap >>= 1;
			}
			// clip left invisible part
			if(d - (cb*cbd) >= s2 || d < s1) {
				d -= cb * cbd;
				if(need_correct_s)
					s += cb;
				continue;
			} else if(d >= s2) {
				unsigned char sk = (d - s2) / cbd;
				d -= sk * cbd;
				if(need_correct_s)
					s += sk;
				cb -= sk;
			}
			// visible part
			if(ap == alpha) {
				if(ap == 0xFF) {
					do {
						if(d < s1)
							break;
						*((color*)d) = pallette[*s++];
						d -= cbd;
					} while(--cb);
				} else {
					do {
						if(d < s1)
							break;
						unsigned char* s1 = (unsigned char*)&pallette[*s++];
						d[0] = (((int)d[0] * (255 - ap)) + ((s1[0])*(ap))) >> 8;
						d[1] = (((int)d[1] * (255 - ap)) + ((s1[1])*(ap))) >> 8;
						d[2] = (((int)d[2] * (255 - ap)) + ((s1[2])*(ap))) >> 8;
						d -= cbd;
					} while(--cb);
				}
			} else if(ap == 0x7F) {
				do {
					if(d < s1)
						break;
					d[0] >>= 1;
					d[1] >>= 1;
					d[2] >>= 1;
					d -= cbd;
				} while(--cb);
			} else {
				ap = 255 - ap;
				do {
					if(d < s1)
						break;
					d[0] = (((int)d[0] * ap)) >> 8;
					d[1] = (((int)d[1] * ap)) >> 8;
					d[2] = (((int)d[2] * ap)) >> 8;
					d -= cbd;
				} while(--cb);
			}
			// right clip part
			if(cb) {
				if(need_correct_s)
					s += cb;
				d -= cb * cbd;
			}
		} else {
			if(c == 0xA0)
				d -= (*s++)*cbd;
			else
				d -= (c - 0xA0)*cbd;
		}
	}
}

static void alc32(unsigned char* d, int d_scan, const unsigned char* s, int height, const unsigned char* clip_x1, const unsigned char* clip_x2, color c1, bool italic) {
	const int cbs = 3;
	const int cbd = 4;
	unsigned char* p = d;
	while(true) {
		unsigned char c = *s++;
		if(c == 0) {
			d += d_scan;
			clip_x1 += d_scan;
			clip_x2 += d_scan;
			if(italic && (height & 1) != 0)
				d -= cbd;
			p = d;
			if(--height == 0)
				break;
		} else if(c <= 0x7F) {
			// clip left invisible part
			if(p + (c*cbd) <= clip_x1 || p > clip_x2) {
				p += c * cbd;
				s += c * cbs;
				continue;
			} else if(p < clip_x1) {
				unsigned char sk = (clip_x1 - p) / cbd;
				p += sk * cbd;
				s += sk * cbs;
				c -= sk;
			}
			// visible part
			do {
				if(p >= clip_x2)
					break;
				p[0] = ((p[0] * (255 - s[0])) + (c1.b*(s[0]))) >> 8;
				p[1] = ((p[1] * (255 - s[1])) + (c1.g*(s[1]))) >> 8;
				p[2] = ((p[2] * (255 - s[2])) + (c1.r*(s[2]))) >> 8;
				p += cbd;
				s += cbs;
			} while(--c);
			// right clip part
			if(c) {
				p += c * cbd;
				s += c * cbs;
			}
		} else {
			if(c == 0x80)
				p += (*s++)*cbd;
			else
				p += (c - 0x80)*cbd;
		}
	}
}

static unsigned char* skip_v3(unsigned char* s, int h) {
	const int		cbs = 1;
	const int		cbd = 1;
	if(!s || !h)
		return s;
	while(true) {
		unsigned char c = *s++;
		if(c == 0) {
			if(--h == 0)
				return s;
		} else if(c <= 0x9F) {
			if(c <= 0x7F)
				s += c * cbs;
			else {
				if(c == 0x80)
					c = *s++;
				else
					c -= 0x80;
				s++;
				s += c * cbs;
			}
		} else if(c == 0xA0)
			s++;
	}
}

static unsigned char* skip_rle32(unsigned char* s, int h) {
	const int cbs = 3;
	if(!s || !h)
		return s;
	while(true) {
		unsigned char c = *s++;
		if(c == 0) {
			if(--h == 0)
				return s;
		} else if(c <= 0x9F) {
			if(c <= 0x7F)
				s += c * cbs;
			else {
				if(c == 0x80)
					c = *s++;
				else
					c -= 0x80;
				s++;
				s += c * cbs;
			}
		} else if(c == 0xA0)
			s++;
	}
}

static unsigned char* skip_alc(unsigned char* s, int h) {
	const int cbs = 3;
	if(!s || !h)
		return s;
	while(true) {
		unsigned char c = *s++;
		if(c == 0) {
			if(--h == 0)
				return s;
		} else if(c <= 0x7F)
			s += c * cbs;
		else if(c == 0x80)
			s++;
	}
}

static void scale_line_32(unsigned char* dst, unsigned char* src, int sw, int tw) {
	const int cbd = 4;
	int NumPixels = tw;
	int IntPart = (sw / tw)*cbd;
	int FractPart = sw % tw;
	int E = 0;
	while(NumPixels-- > 0) {
		*((unsigned*)dst) = *((unsigned*)src);
		dst += cbd;
		src += IntPart;
		E += FractPart;
		if(E >= tw) {
			E -= tw;
			src += cbd;
		}
	}
}

static bool corrects(const surface& dc, int& x, int& y, int& width, int& height) {
	if(x + width > dc.width)
		width = dc.width - x;
	if(y + height > dc.height)
		height = dc.height - y;
	if(width <= 0 || height <= 0)
		return false;
	return true;
}

static bool correctb(int& x1, int& y1, int& w, int& h, int& ox) {
	int x11 = x1;
	int x2 = x1 + w;
	int y2 = y1 + h;
	if(!correct(x1, y1, x2, y2, draw::clipping))
		return false;
	ox = x1 - x11;
	w = x2 - x1;
	h = y2 - y1;
	return true;
}

static void scale32(
	unsigned char* d, int d_scan, int d_width, int d_height,
	unsigned char* s, int s_scan, int s_width, int s_height) {
	if(!d_width || !d_height || !s_width || !s_height)
		return;
	const int cbd = 4;
	int NumPixels = d_height;
	int IntPart = (s_height / d_height) * s_scan;
	int FractPart = s_height % d_height;
	int E = 0;
	unsigned char* PrevSource = 0;
	while(NumPixels-- > 0) {
		if(s == PrevSource)
			memcpy(d, d - d_scan, d_width*cbd);
		else {
			scale_line_32(d, s, s_width, d_width);
			PrevSource = s;
		}
		d += d_scan;
		s += IntPart;
		E += FractPart;
		if(E >= d_height) {
			E -= d_height;
			s += s_scan;
		}
	}
}

static void cpy(unsigned char* d, int d_scan, unsigned char* s, int s_scan, int width, int height, int bytes_per_pixel) {
	if(height <= 0 || width <= 0)
		return;
	int width_bytes = width * bytes_per_pixel;
	do {
		memcpy(d, s, width_bytes);
		s += s_scan;
		d += d_scan;
	} while(--height);
}

static void cpy32t(unsigned char* d, int d_scan, unsigned char* s, int s_scan, int width, int height) {
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

draw::state::state() :
	fore(draw::fore),
	fore_stroke(draw::fore_stroke),
	font(draw::font),
	linw(draw::linw),
	canvas(draw::canvas),
	clip(clipping) {
}

draw::state::~state() {
	draw::font = this->font;
	draw::fore = this->fore;
	draw::fore_stroke = this->fore_stroke;
	draw::linw = this->linw;
	draw::clipping = this->clip;
	draw::canvas = this->canvas;
}

rect draw::getarea() {
	return sys_static_area;
}

void draw::dragbegin(const void* p) {
	drag_object = p;
	dragmouse = hot.mouse;
}

bool draw::dragactive() {
	return drag_object != 0;
}

bool draw::dragactive(const void* p) {
	if(drag_object == p) {
		if(!hot.pressed || hot.key == KeyEscape) {
			drag_object = 0;
			hot.zero();
			hot.cursor = CursorArrow;
			return false;
		}
		return true;
	}
	return false;
}

int draw::getbpp() {
	return canvas ? canvas->bpp : 1;
}

int draw::getwidth() {
	return canvas ? canvas->width : 0;
}

int draw::getheight() {
	return canvas ? canvas->height : 0;
}

unsigned char* draw::ptr(int x, int y) {
	return canvas ? (canvas->bits + y * canvas->scanline + x * canvas->bpp / 8) : 0;
}

color draw::getcolor(color normal, unsigned flags) {
	if(flags&Disabled)
		return normal.mix(colors::window);
	return normal;
}

color draw::getcolor(rect rc, color normal, color active, unsigned flags) {
	if(flags&Disabled)
		return normal.mix(colors::window);
	if(areb(rc))
		return active;
	return normal;
}

void draw::decortext(unsigned flags) {
	draw::fore = getcolor(colors::text, flags);
}

void draw::pixel(int x, int y) {
	if(x >= clipping.x1 && x < clipping.x2 && y >= clipping.y1 && y < clipping.y2) {
		if(!canvas)
			return;
		*((color*)((char*)canvas->bits + y * canvas->scanline + x * 4)) = fore;
	}
}

void draw::pixel(int x, int y, unsigned char a) {
	if(x < clipping.x1 || x >= clipping.x2 || y < clipping.y1 || y >= clipping.y2 || a == 0xFF)
		return;
	color* p = (color*)ptr(x, y);
	if(a == 0)
		*p = fore;
	else {
		p->b = (((unsigned)p->b*(a)) + (fore.b*(255 - a))) >> 8;
		p->g = (((unsigned)p->g*(a)) + (fore.g*(255 - a))) >> 8;
		p->r = (((unsigned)p->r*(a)) + (fore.r*(255 - a))) >> 8;
		p->a = 0;
	}
}

static void linew(int x0, int y0, int x1, int y1, float wd) {
	int dx = iabs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = iabs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = dx - dy, e2, x2, y2; /* error value e_xy */
	float ed = dx + dy == 0 ? 1 : sqrt((float)dx*dx + (float)dy*dy);

	for(wd = (wd + 1) / 2; ; ) {                                    /* pixel loop */
		draw::pixel(x0, y0, (unsigned char)imax((int)0, (int)(255 * (iabs(err - dx + dy) / ed - wd + 1))));
		e2 = err; x2 = x0;
		if(2 * e2 >= -dx) {                                            /* x step */
			for(e2 += dy, y2 = y0; e2 < ed*wd && (y1 != y2 || dx > dy); e2 += dx)
				draw::pixel(x0, y2 += sy, (unsigned char)imax((int)0, (int)(255 * (iabs(e2) / ed - wd + 1))));
			if(x0 == x1) break;
			e2 = err; err -= dy; x0 += sx;
		}
		if(2 * e2 <= dy) {                                             /* y step */
			for(e2 = dx - e2; e2 < ed*wd && (x1 != x2 || dx < dy); e2 += dy)
				draw::pixel(x2 += sx, y0, (unsigned char)imax((int)0, (int)(255 * (iabs(e2) / ed - wd + 1))));
			if(y0 == y1) break;
			err += dx; y0 += sy;
		}
	}
}

void draw::line(int x0, int y0, int x1, int y1) {
	if(!canvas)
		return;
	if(linw != 1.0)
		linew(x0, y0, x1, y1, linw);
	else if(y0 == y1) {
		if(!correct(x0, y0, x1, y1, clipping, false))
			return;
		set32(canvas->ptr(x0, y0), canvas->scanline, x1 - x0 + 1, 1, set32);
	} else if(x0 == x1) {
		if(!correct(x0, y0, x1, y1, clipping, false))
			return;
		set32(canvas->ptr(x0, y0), canvas->scanline, 1, y1 - y0 + 1, set32);
	} else if(line_antialiasing) {
		int dx = iabs(x1 - x0), sx = x0 < x1 ? 1 : -1;
		int dy = iabs(y1 - y0), sy = y0 < y1 ? 1 : -1;
		int err = dx - dy, e2, x2; // error value e_xy
		int ed = dx + dy == 0 ? 1 : isqrt(dx*dx + dy * dy);
		for(;;) {
			pixel(x0, y0, 255 * iabs(err - dx + dy) / ed);
			e2 = err; x2 = x0;
			if(2 * e2 >= -dx) // x step
			{
				if(x0 == x1) break;
				if(e2 + dy < ed)
					pixel(x0, y0 + sy, 255 * (e2 + dy) / ed);
				err -= dy; x0 += sx;
			}
			if(2 * e2 <= dy) // y step
			{
				if(y0 == y1)
					break;
				if(dx - e2 < ed)
					pixel(x2 + sx, y0, 255 * (dx - e2) / ed);
				err += dx; y0 += sy;
			}
		}
	} else {
		int dx = iabs(x1 - x0), sx = x0 < x1 ? 1 : -1;
		int dy = -iabs(y1 - y0), sy = y0 < y1 ? 1 : -1;
		int err = dx + dy, e2;
		for(;;) {
			pixel(x0, y0);
			e2 = 2 * err;
			if(e2 >= dy) {
				if(x0 == x1) break;
				err += dy; x0 += sx;
			}
			if(e2 <= dx) {
				if(y0 == y1) break;
				err += dx; y0 += sy;
			}
		}
	}
}

void draw::bezierseg(int x0, int y0, int x1, int y1, int x2, int y2) {
	int sx = x2 - x1, sy = y2 - y1;
	long xx = x0 - x1, yy = y0 - y1, xy;             /* relative values for checks */
	double dx, dy, err, ed, cur = xx * sy - yy * sx;    /* curvature */
	assert(xx*sx <= 0 && yy*sy <= 0);				/* sign of gradient must not change */
	if(sx*(long)sx + sy * (long)sy > xx*xx + yy * yy) { /* begin with longer part */
		x2 = x0; x0 = sx + x1; y2 = y0; y0 = sy + y1; cur = -cur;     /* swap P0 P2 */
	}
	if(cur != 0) {                                                      /* no straight line */
		xx += sx; xx *= sx = x0 < x2 ? 1 : -1;              /* x step direction */
		yy += sy; yy *= sy = y0 < y2 ? 1 : -1;              /* y step direction */
		xy = 2 * xx*yy; xx *= xx; yy *= yy;             /* differences 2nd degree */
		if(cur*sx*sy < 0) {                              /* negated curvature? */
			xx = -xx; yy = -yy; xy = -xy; cur = -cur;
		}
		dx = 4.0*sy*(x1 - x0)*cur + xx - xy;                /* differences 1st degree */
		dy = 4.0*sx*(y0 - y1)*cur + yy - xy;
		xx += xx; yy += yy; err = dx + dy + xy;                   /* error 1st step */
		do {
			cur = imin(dx + xy, -xy - dy);
			ed = imax(dx + xy, -xy - dy);               /* approximate error distance */
			ed += 2 * ed*cur*cur / (4 * ed*ed + cur * cur);
			pixel(x0, y0, (unsigned char)(255 * iabs(err - dx - dy - xy) / ed));          /* plot curve */
			if(x0 == x2 || y0 == y2) break;     /* last pixel -> curve finished */
			x1 = x0; cur = dx - err; y1 = 2 * err + dy < 0;
			if(2 * err + dx > 0) {                                        /* x step */
				if(err - dy < ed) pixel(x0, y0 + sy, (unsigned char)(255 * iabs(err - dy) / ed));
				x0 += sx; dx -= xy; err += dy += yy;
			}
			if(y1) {                                                  /* y step */
				if(cur < ed) pixel(x1 + sx, y0, (unsigned char)(255 * iabs(cur) / ed));
				y0 += sy; dy -= xy; err += dx += xx;
			}
		} while(dy < dx);                  /* gradient negates -> close curves */
	}
	line(x0, y0, x2, y2);                  /* plot remaining needle to end */
}

void draw::bezier(int x0, int y0, int x1, int y1, int x2, int y2) {
	int x = x0 - x1, y = y0 - y1;
	double t = x0 - 2 * x1 + x2, r;
	if((long)x*(x2 - x1) > 0) {                        /* horizontal cut at P4? */
		if((long)y*(y2 - y1) > 0)                     /* vertical cut at P6 too? */
			if(iabs((y0 - 2 * y1 + y2) / t * x) > iabs(y)) {               /* which first? */
				x0 = x2; x2 = x + x1; y0 = y2; y2 = y + y1;            /* swap points */
			}                            /* now horizontal cut at P4 comes first */
		t = (x0 - x1) / t;
		r = (1 - t)*((1 - t)*y0 + 2.0*t*y1) + t * t*y2;                       /* By(t=P4) */
		t = (x0*x2 - x1 * x1)*t / (x0 - x1);                       /* gradient dP4/dx=0 */
		x = ifloor(t + 0.5); y = ifloor(r + 0.5);
		r = (y1 - y0)*(t - x0) / (x1 - x0) + y0;                  /* intersect P3 | P0 P1 */
		bezierseg(x0, y0, x, ifloor(r + 0.5), x, y);
		r = (y1 - y2)*(t - x2) / (x1 - x2) + y2;                  /* intersect P4 | P1 P2 */
		x0 = x1 = x; y0 = y; y1 = ifloor(r + 0.5);             /* P0 = P4, P1 = P8 */
	}
	if((long)(y0 - y1)*(y2 - y1) > 0) {                    /* vertical cut at P6? */
		t = y0 - 2 * y1 + y2; t = (y0 - y1) / t;
		r = (1 - t)*((1 - t)*x0 + 2.0*t*x1) + t * t*x2;                       /* Bx(t=P6) */
		t = (y0*y2 - y1 * y1)*t / (y0 - y1);                       /* gradient dP6/dy=0 */
		x = ifloor(r + 0.5); y = ifloor(t + 0.5);
		r = (x1 - x0)*(t - y0) / (y1 - y0) + x0;                  /* intersect P6 | P0 P1 */
		bezierseg(x0, y0, ifloor(r + 0.5), y, x, y);
		r = (x1 - x2)*(t - y2) / (y1 - y2) + x2;                  /* intersect P7 | P1 P2 */
		x0 = x; x1 = ifloor(r + 0.5); y0 = y1 = y;             /* P0 = P6, P1 = P7 */
	}
	bezierseg(x0, y0, x1, y1, x2, y2);                  /* remaining part */
}

void draw::spline(point* original_points, int n) {
	point points[256];
	n = imin(sizeof(points) / sizeof(points[0]), (unsigned)n) - 1;
	memcpy(points, original_points, sizeof(points[0])*(n + 1));
	const int M_MAX = 6;
	float mi = 1, m[M_MAX];                    /* diagonal constants of matrix */
	int i, x0, y0, x1, y1, x2 = points[n].x, y2 = points[n].y;
	assert(n > 1);                        /* need at least 3 points P[0]..P[n] */
	points[1].x = x0 = 8 * points[1].x - 2 * points[0].x;                          /* first row of matrix */
	points[1].y = y0 = 8 * points[1].y - 2 * points[0].y;
	for(i = 2; i < n; i++) {                                 /* forward sweep */
		if(i - 2 < M_MAX) m[i - 2] = mi = (float)(1.0 / (6.0 - mi));
		points[i].x = x0 = ifloor(8 * points[i].x - x0 * mi + 0.5);                        /* store yi */
		points[i].y = y0 = ifloor(8 * points[i].y - y0 * mi + 0.5);
	}
	x1 = ifloor((x0 - 2 * x2) / (5.0 - mi) + 0.5);                 /* correction last row */
	y1 = ifloor((y0 - 2 * y2) / (5.0 - mi) + 0.5);
	for(i = n - 2; i > 0; i--) {                           /* back substitution */
		if(i <= M_MAX) mi = m[i - 1];
		x0 = ifloor((points[i].x - x1)*mi + 0.5);                            /* next corner */
		y0 = ifloor((points[i].y - y1)*mi + 0.5);
		bezier((x0 + x1) / 2, (y0 + y1) / 2, x1, y1, x2, y2);
		x2 = (x0 + x1) / 2; x1 = x0;
		y2 = (y0 + y1) / 2; y1 = y0;
	}
	bezier(points[0].x, points[0].y, x1, y1, x2, y2);
}

void draw::line(int x0, int y0, int x1, int y1, color c1) {
	auto push_fore = fore; fore = c1;
	line(x0, y0, x1, y1);
	fore = push_fore;
}

void draw::linet(int x0, int y0, int x1, int y1) {
	int dx = iabs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = -iabs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = dx + dy, e2; // error value e_xy
	for(;;) {
		if((x0 + y0) & 1)
			pixel(x0, y0);
		if(x0 == x1 && y0 == y1) break;
		e2 = 2 * err;
		if(e2 >= dy) { err += dy; x0 += sx; } // e_xy+e_x > 0
		if(e2 <= dx) { err += dx; y0 += sy; } // e_xy+e_y < 0
	}
}

static void triangle_bottom(point v1, point v2, point v3) {
	auto invslope1 = float(v2.x - v1.x) / float(v2.y - v1.y);
	auto invslope2 = float(v3.x - v1.x) / float(v3.y - v1.y);
	float curx1 = v1.x;
	float curx2 = v1.x;
	for(int scanlineY = v1.y; scanlineY <= v2.y; scanlineY++) {
		line((int)curx1, scanlineY, (int)curx2, scanlineY);
		curx1 += invslope1;
		curx2 += invslope2;
	}
}

static void triangle_top(point v1, point v2, point v3) {
	float invslope1 = float(v3.x - v1.x) / float(v3.y - v1.y);
	float invslope2 = float(v3.x - v2.x) / float(v3.y - v2.y);
	float curx1 = v3.x;
	float curx2 = v3.x;
	for(int scanlineY = v3.y; scanlineY > v1.y; scanlineY--) {
		line((int)curx1, scanlineY, (int)curx2, scanlineY);
		curx1 -= invslope1;
		curx2 -= invslope2;
	}
}

void draw::triangle(point v1, point v2, point v3, color c1) {
	auto pf = fore; fore = c1;
	if(v2.y < v1.y)
		iswap(v1, v2);
	if(v3.y < v1.y)
		iswap(v1, v3);
	if(v3.y < v2.y)
		iswap(v2, v3);
	if(v2.y == v3.y)
		triangle_bottom(v1, v2, v3);
	else if(v1.y == v2.y)
		triangle_top(v1, v2, v3);
	else {
		point v4 = {(short)(v1.x + ((float)(v2.y - v1.y) / (float)(v3.y - v1.y)) * (v3.x - v1.x)), (short)v2.y};
		triangle_bottom(v1, v2, v4);
		triangle_top(v2, v4, v3);
	}
	fore = pf;
}

void draw::rectb(rect rc) {
	line(rc.x1, rc.y1, rc.x2, rc.y1);
	line(rc.x2, rc.y1 + 1, rc.x2, rc.y2);
	line(rc.x2 - 1, rc.y2, rc.x1, rc.y2);
	line(rc.x1, rc.y2 - 1, rc.x1, rc.y1);
}

void draw::rectb(rect rc, color c1) {
	auto push_fore = fore; fore = c1;
	rectb(rc);
	fore = push_fore;
}

void draw::rectf(rect rc) {
	if(!canvas)
		return;
	if(!correct(rc.x1, rc.y1, rc.x2, rc.y2, clipping))
		return;
	if(rc.x1 == rc.x2)
		return;
	set32(ptr(rc.x1, rc.y1), canvas->scanline, rc.x2 - rc.x1, rc.y2 - rc.y1, set32);
}

void draw::rectf(rect rc, color c1) {
	auto push_fore = fore; fore = c1;
	rectf(rc);
	fore = push_fore;
}

void draw::rectf(rect rc, color c1, unsigned char alpha) {
	if(!canvas)
		return;
	if(!correct(rc.x1, rc.y1, rc.x2, rc.y2, clipping))
		return;
	if(rc.x1 == rc.x2)
		return;
	auto pf = fore;
	fore = c1;
	fore.a = alpha;
	set32(ptr(rc.x1, rc.y1), canvas->scanline, rc.x2 - rc.x1, rc.y2 - rc.y1, set32a);
	fore = pf;
}

void draw::rectx(rect rc, color c1) {
	auto push_fore = fore; fore = c1;
	linet(rc.x1, rc.y1, rc.x2, rc.y1);
	linet(rc.x2, rc.y1 + 1, rc.x2, rc.y2);
	linet(rc.x2 - 1, rc.y2, rc.x1, rc.y2);
	linet(rc.x1, rc.y2 - 1, rc.x1, rc.y1);
	fore = push_fore;
}

void draw::gradv(rect rc, const color c1, const color c2, int skip) {
	if(!canvas)
		return;
	int m = iabs(rc.height());
	if(skip > m)
		skip = m;
	double k3 = m;
	if(!k3)
		return;
	int y0 = rc.y1;
	if(!correct(rc.x1, rc.y1, rc.x2, rc.y2, clipping))
		return;
	int w1 = rc.width();
	skip += rc.y1 - y0;
	auto pf = fore;
	for(int y = rc.y1 + skip; y < rc.y2; y++) {
		double k2 = (double)(y - rc.y1) / k3;
		double k1 = 1.00f - k2;
		fore.r = (unsigned char)(c1.r*k1 + c2.r*k2);
		fore.g = (unsigned char)(c1.g*k1 + c2.g*k2);
		fore.b = (unsigned char)(c1.b*k1 + c2.b*k2);
		set32(canvas->ptr(rc.x1, y), canvas->scanline, w1, 1, set32);

	}
	fore = pf;
}

void draw::gradh(rect rc, const color c1, const color c2, int skip) {
	if(!canvas)
		return;
	double k3 = iabs(rc.width());
	if(!k3)
		return;
	int x0 = rc.x1;
	if(!correct(rc.x1, rc.y1, rc.x2, rc.y2, clipping))
		return;
	int h1 = rc.height();
	skip += rc.x1 - x0;
	auto pf = fore;
	for(int x = rc.x1 + skip; x < rc.x2; x++) {
		double k2 = (double)(x - rc.x1) / k3;
		double k1 = 1.00f - k2;
		fore.r = (unsigned char)(c1.r*k1 + c2.r*k2);
		fore.g = (unsigned char)(c1.g*k1 + c2.g*k2);
		fore.b = (unsigned char)(c1.b*k1 + c2.b*k2);
		set32(canvas->ptr(x, rc.y1), canvas->scanline, 1, h1, set32);
	}
	fore = pf;
}

void draw::circlef(int xm, int ym, int r, const color c1, unsigned char alpha) {
	if(xm - r >= clipping.x2 || xm + r < clipping.x1 || ym - r >= clipping.y2 || ym + r < clipping.y1)
		return;
	int x = -r, y = 0, err = 2 - 2 * r, y1, y2 = -1000;
	do {
		y1 = ym + y;
		if(y2 != y) {
			y2 = y;
			rectf({xm + x, y1, xm - x, y1 + 1}, c1, alpha);
			if(y != 0) {
				y1 = ym - y;
				rectf({xm + x, y1, xm - x, y1 + 1}, c1, alpha);
			}
		}
		r = err;
		if(r <= y)
			err += ++y * 2 + 1;
		if(r > x || err > y)
			err += ++x * 2 + 1;
	} while(x < 0);
}

void draw::circle(int xm, int ym, int r) {
	int x = r, y = 0; // II. quadrant from bottom left to top right
	int x2, e2, err = 2 - 2 * r; // error of 1.step
	r = 1 - err;
	for(;;) {
		int i = 255 * iabs(err + 2 * (x + y) - 2) / r; // get blend value of pixel
		pixel(xm + x, ym - y, i); // I. Quadrant
		pixel(xm + y, ym + x, i); // II. Quadrant
		pixel(xm - x, ym + y, i); // III. Quadrant
		pixel(xm - y, ym - x, i); // IV. Quadrant
		if(x == 0)
			break;
		e2 = err; x2 = x; // remember values
		if(err > y) {
			/* x step */
			int i = 255 * (err + 2 * x - 1) / r; // outward pixel
			if(i < 255) {
				pixel(xm + x, ym - y + 1, i);
				pixel(xm + y - 1, ym + x, i);
				pixel(xm - x, ym + y - 1, i);
				pixel(xm - y + 1, ym - x, i);
			}
			err -= --x * 2 - 1;
		}
		if(e2 <= x2--) {
			/* y step */
			int i = 255 * (1 - 2 * y - e2) / r;
			if(i < 255) {
				pixel(xm + x2, ym - y, i);
				pixel(xm + y, ym + x2, i);
				pixel(xm - x2, ym + y, i);
				pixel(xm - y, ym - x2, i);
			}
			err -= --y * 2 - 1;
		}
	}
}

void draw::circle(int x, int y, int r, const color c1) {
	auto fore_push = fore; fore = c1;
	circle(x, y, r);
	fore = fore_push;
}

void draw::setclip(rect rcn) {
	rect rc = draw::clipping;
	if(rc.x1 < rcn.x1)
		rc.x1 = rcn.x1;
	if(rc.y1 < rcn.y1)
		rc.y1 = rcn.y1;
	if(rc.x2 > rcn.x2)
		rc.x2 = rcn.x2;
	if(rc.y2 > rcn.y2)
		rc.y2 = rcn.y2;
	draw::clipping = rc;
}

static void intersect_rect(rect& r1, const rect& r2) {
	if(!r1.intersect(r2))
		return;
	if(hot.mouse.in(r2)) {
		if(r2.y1 > r1.y1)
			r1.y1 = r2.y1;
		if(r2.x1 > r1.x1)
			r1.x1 = r2.x1;
		if(r2.y2 < r1.y2)
			r1.y2 = r2.y2;
		if(r2.x2 < r1.x2)
			r1.x2 = r2.x2;
	} else {
		if(hot.mouse.y > r2.y2 && r2.y2 > r1.y1)
			r1.y1 = r2.y2;
		else if(hot.mouse.y < r2.y1 && r2.y1 < r1.y2)
			r1.y2 = r2.y1;
		else if(hot.mouse.x > r2.x2 && r2.x2 > r1.x1)
			r1.x1 = r2.x2;
		else if(hot.mouse.x < r2.x1 && r2.x1 < r1.x2)
			r1.x2 = r2.x1;
	}
}

areas draw::area(rect rc) {
	if(sys_optimize_mouse_move)
		intersect_rect(sys_static_area, rc);
	if(dragactive())
		return AreaNormal;
	if(!hot.mouse.in(clipping))
		return AreaNormal;
	if(!mouseinput)
		return AreaNormal;
	if(hot.mouse.in(rc))
		return hot.pressed ? AreaHilitedPressed : AreaHilited;
	return AreaNormal;
}

bool draw::areb(rect rc) {
	areas a = area(rc);
	return a == AreaHilited
		|| a == AreaHilitedPressed;
}

int	draw::aligned(int x, int width, unsigned flags, int dx) {
	switch(flags&AlignMask) {
	case AlignRightBottom:
	case AlignRightCenter:
	case AlignRight: return x + width - dx;
	case AlignCenterBottom:
	case AlignCenterCenter:
	case AlignCenter: return x + (width - dx) / 2;
	default: return x;
	}
}

int draw::alignedh(const rect& rc, const char* string, unsigned state) {
	int ty;
	switch(state&AlignMask) {
	case AlignCenterCenter:
	case AlignRightCenter:
	case AlignLeftCenter:
		if(state&TextSingleLine)
			ty = draw::texth();
		else
			ty = draw::texth(string, rc.width());
		return (rc.height() - ty) / 2;
	case AlignCenterBottom:
	case AlignRightBottom:
	case AlignLeftBottom:
		if(state&TextSingleLine)
			ty = draw::texth();
		else
			ty = draw::texth(string, rc.width());
		return rc.y2 - ty;
	default:
		return 0;
	}
}

int draw::textw(const char* string, int count) {
	if(!font)
		return 0;
	int x1 = 0;
	if(count == -1) {
		const char *s1 = string;
		while(*s1)
			x1 += textw(szget(&s1));
	} else {
		const char *s1 = string;
		const char *s2 = string + count;
		while(s1 < s2)
			x1 += textw(szget(&s1));
	}
	return x1;
}

const char* draw::skiptr(const char* string) {
	// skiping trail symbols
	for(; *string && *string == 0x20; string++);
	if(*string == 13) {
		string++;
		if(*string == 10)
			string++;
	} else if(*string == 10) {
		string++;
		if(*string == 13)
			string++;
	}
	return string;
}

int draw::textw(rect& rc, const char* string) {
	int w1 = rc.width();
	rc.y2 = rc.y1;
	rc.x2 = rc.x1;
	while(string[0]) {
		int c = textbc(string, w1);
		if(!c)
			break;
		int m = textw(string, c);
		if(rc.width() < m)
			rc.x2 = rc.x1 + m;
		rc.y2 += texth();
		string = skiptr(string + c);
	}
	return rc.height();
}

int draw::texth(const char* string, int width) {
	int dy = texth();
	int y1 = 0;
	while(*string) {
		int c = textbc(string, width);
		if(!c)
			break;
		y1 += dy;
		string = skiptr(string + c);
	}
	return y1;
}

void draw::text(int x, int y, const char* string, int count, unsigned flags) {
	if(!font)
		return;
	int dy = texth();
	if(y >= clipping.y2 || y + dy < clipping.y1)
		return;
	if(count == -1)
		count = zlen(string);
	const char *s1 = string;
	const char *s2 = string + count;
	while(s1 < s2) {
		int sm = szget(&s1);
		if(sm >= 0x21)
			glyph(x, y, sm, flags);
		x += textw(sm);
	}
}

int draw::textc(int x, int y, int width, const char* string, int count, unsigned flags, bool* clipped) {
	state push;
	setclip({x, y, x + width, y + texth()});
	auto w = textw(string, count);
	if(clipped)
		*clipped = w > width;
	text(aligned(x, width, flags, w), y, string, count, flags);
	return texth();
}

int draw::textbc(const char* string, int width) {
	if(!font)
		return 0;
	int p = -1;
	int w = 0;
	const char* s1 = string;
	while(true) {
		unsigned s = szget(&s1);
		if(s == 0x20 || s == 9)
			p = s1 - string;
		else if(s == 0) {
			p = s1 - string - 1;
			break;
		} else if(s == 10 || s == 13) {
			p = s1 - string;
			break;
		}
		w += textw(s);
		if(w > width)
			break;
	}
	if(p == -1)
		p = s1 - string;
	return p;
}

int	draw::text(rect rc, const char* string, unsigned state, int* max_width) {
	if(!string || string[0] == 0)
		return 0;
	int x1 = rc.x1;
	int y1 = rc.y1 + alignedh(rc, string, state);
	int dy = texth();
	if(max_width)
		*max_width = 0;
	if(state&TextSingleLine) {
		draw::state push;
		draw::setclip(rc);
		text(aligned(x1, rc.width(), state, draw::textw(string)), y1,
			string, -1, state);
		return dy;
	} else {
		int w1 = rc.width();
		while(y1 < rc.y2) {
			int c = textbc(string, w1);
			if(!c)
				break;
			int w = textw(string, c);
			if(max_width && *max_width < w)
				*max_width = w;
			text(aligned(x1, w1, state, w), y1, string, c, state);
			y1 += dy;
			string = skiptr(string + c);
		}
		return y1 - rc.y1;
	}
}

int draw::textlb(const char* string, int index, int width, int* line_index, int* line_count) {
	auto dy = texth();
	auto p = string;
	if(line_index)
		*line_index = 0;
	if(line_count)
		*line_count = 0;
	while(true) {
		int c = textbc(p, width);
		if(!c)
			break;
		if(index < c || p[c] == 0) {
			if(line_count)
				*line_count = c;
			break;
		}
		index -= c;
		if(line_index)
			*line_index = *line_index + 1;
		p = skiptr(p + c);
	}
	return p - string;
}

static void hilite_text_line(int x, int y, int width, int height, const char* string, int count, unsigned state, int i1, int i2) {
	int w = draw::textw(string, count);
	auto x0 = draw::aligned(x, width, state, w);
	// Выделение
	if(i1 != i2 && ((i1 >= 0 && i1 < count) || (i2 >= 0 && i2 < count) || (i1 < 0 && i2 >= count))) {
		int rx1 = x0 + 1;
		int rx2 = x0 + w + 1;
		if(i1 >= 0 && i1 <= count)
			rx1 = x0 + 1 + draw::textw(string, i1);
		if(i2 >= 0 && i2 <= count)
			rx2 = x0 + 1 + draw::textw(string, i2);
		draw::rectf({rx1, y, rx2, y + height}, colors::edit);
	}
	// Вывод текста
	draw::text(x0, y, string, count);
	// Вывод курсора
	if(i1 >= 0 && i1 == i2
		&& (i1 < count || (i1 == count && string[count] == 0))) {
		int rx1 = x0 + draw::textw(string, i1);
		draw::line(rx1, y, rx1, y + height, colors::text);
	}
}

int	draw::texte(rect rc, const char* string, unsigned state, int p1, int p2) {
	auto i1 = p1;
	auto i2 = p2;
	if(i2 != -1 && i1 > i2)
		iswap(i1, i2);
	if(i2 == -1)
		i2 = i1;
	auto dy = texth();
	auto w1 = rc.width();
	auto y1 = rc.y1 + alignedh(rc, string, state);
	auto p = string;
	if(!p)
		return 0;
	if(state&TextSingleLine) {
		hilite_text_line(rc.x1, y1, w1, dy, p, zlen(p), state, i1, i2);
		return dy;
	} else if(string[0]) {
		while(y1 < rc.y2) {
			int c = textbc(p, w1);
			if(!c)
				break;
			hilite_text_line(rc.x1, y1, w1, dy, p, c, state, i1, i2);
			i1 -= c;
			i2 -= c;
			y1 += dy;
			p = skiptr(p + c);
		}
	} else
		hilite_text_line(rc.x1, y1, w1, dy, p, 0, state, i1, i2);
	return y1 - rc.y1;
}

int draw::hittest(int x, int hit_x, const char* p, int lenght) {
	if(hit_x < x)
		return -2;
	int index = 0;
	int syw = 0;
	while(index < lenght) {
		syw = draw::textw(szget(&p));
		if(hit_x <= x + 1 + syw / 2)
			break;
		x += syw;
		index++;
	}
	if(index == lenght) {
		if(hit_x > x + syw)
			index = -3;
	}
	return index;
}

int draw::hittest(rect rc, const char* string, unsigned state, point pt) {
	int w1 = rc.width();
	int dy = texth();
	int x1 = rc.x1;
	int y1 = rc.y1 + draw::alignedh(rc, string, state);
	auto p = (const char*)string;
	if(state&TextSingleLine) {
		auto c = zlen(string);
		auto w1 = textw(string, c);
		auto x0 = aligned(x1, w1, state, w1);
		return hittest(x0, pt.x, string, c);
	} else {
		while(y1 < rc.y2) {
			int c = textbc(p, w1);
			if(!c)
				break;
			int w = textw(p, c);
			auto x0 = aligned(x1, w1, state, w);
			if(pt.y >= y1 && pt.y < y1 + dy) {
				if(pt.x >= x0 && pt.x < x0 + w)
					return p - string + draw::hittest(x0, pt.x, p, c);
				if(pt.x < x0)
					return -2;
				return -3;
			}
			y1 += dy;
			p = draw::skiptr(p + c);
		}
	}
	return -1;
}

void draw::image(int x, int y, const sprite* e, int id, int flags, unsigned char alpha) {
	const int cbd = 1;
	int x2, y2;
	color* pal;
	if(!e)
		return;
	const sprite::frame& f = e->get(id);
	if(!f.offset)
		return;
	if(!canvas)
		return;
	if(flags&ImageMirrorH) {
		x2 = x;
		if((flags&ImageNoOffset) == 0)
			x2 += f.ox;
		x = x2 - f.sx;
	} else {
		if((flags&ImageNoOffset) == 0)
			x -= f.ox;
		x2 = x + f.sx;
	}
	if(flags&ImageMirrorV) {
		y2 = y;
		if((flags&ImageNoOffset) == 0)
			y2 += f.oy;
		y = y2 - f.sy;
	} else {
		if((flags&ImageNoOffset) == 0)
			y -= f.oy;
		y2 = y + f.sy;
	}
	unsigned char* s = (unsigned char*)e + f.offset;
	if(y2<clipping.y1 || y>clipping.y2 || x2<clipping.x1 || x>clipping.x2)
		return;
	if(y < clipping.y1) {
		if((flags&ImageMirrorV) == 0) {
			switch(f.encode) {
			case sprite::ALC: s = skip_alc(s, clipping.y1 - y); break;
			case sprite::RAW: s += (clipping.y1 - y)*f.sx * 3; break;
			case sprite::RAW8: s += (clipping.y1 - y)*f.sx; break;
			case sprite::RLE8: s = skip_v3(s, clipping.y1 - y); break;
			case sprite::RLE: s = skip_rle32(s, clipping.y1 - y); break;
			default: break;
			}
		}
		y = clipping.y1;
	}
	if(y2 > clipping.y2) {
		if(flags&ImageMirrorV) {
			switch(f.encode) {
			case sprite::ALC: s = skip_alc(s, y2 - clipping.y2); break;
			case sprite::RAW: s += (y2 - clipping.y2)*f.sx * 3; break;
			case sprite::RAW8: s += (y2 - clipping.y2)*f.sx; break;
			case sprite::RLE8: s = skip_v3(s, y2 - clipping.y2); break;
			case sprite::RLE: s = skip_rle32(s, y2 - clipping.y2); break;
			default: break;
			}
		}
		y2 = clipping.y2;
	}
	if(y >= y2)
		return;
	int wd = (flags&ImageMirrorV) ? -canvas->scanline : canvas->scanline;
	int sy = (flags&ImageMirrorV) ? y2 - 1 : y;
	switch(f.encode) {
	case sprite::RAW:
		if(x < clipping.x1) {
			s += (clipping.x1 - x) * 3;
			x = clipping.x1;
		}
		if(x2 > clipping.x2)
			x2 = clipping.x2;
		if(x >= x2)
			return;
		if(flags&ImageMirrorH)
			raw32m(ptr(x2 - 1, y), wd, s, f.sx * 3,
				x2 - x,
				y2 - y);
		else
			raw32(ptr(x, y), wd, s, f.sx * 3,
				x2 - x,
				y2 - y);
		break;
	case sprite::RAW8:
		if(x < clipping.x1) {
			s += clipping.x1 - x;
			x = clipping.x1;
		}
		if(x2 > clipping.x2)
			x2 = clipping.x2;
		if(x >= x2)
			return;
		if(!f.pallette || (flags&ImagePallette))
			pal = draw::palt;
		else
			pal = (color*)e->ptr(f.pallette);
		if(!pal)
			return;
		if(flags&ImageMirrorH)
			raw832m(ptr(x2 - 1, y), wd, s, f.sx,
				x2 - x,
				y2 - y,
				pal);
		else
			raw832(ptr(x, y), wd, s, f.sx, x2 - x, y2 - y, pal);
		break;
	case sprite::RLE8:
		if(!f.pallette || (flags&ImagePallette))
			pal = draw::palt;
		else
			pal = (color*)e->ptr(f.pallette);
		if(!pal)
			return;
		if(flags&ImageMirrorH)
			rle832m(ptr(x2 - 1, sy), wd, s, y2 - y,
				ptr(clipping.x1, sy),
				ptr(clipping.x2, sy),
				alpha, pal);
		else
			rle832(ptr(x, sy), wd, s, y2 - y,
				ptr(clipping.x1, sy),
				ptr(clipping.x2, sy),
				alpha, pal);
		break;
	case sprite::RLE:
		if(flags&ImageMirrorH)
			rle32m(ptr(x2 - 1, sy), wd, s, y2 - y,
				ptr(clipping.x1, sy),
				ptr(clipping.x2, sy),
				alpha);
		else
			rle32(ptr(x, sy), wd, s, y2 - y,
				ptr(clipping.x1, sy),
				ptr(clipping.x2, sy),
				alpha);
		break;
	case sprite::ALC:
		if(flags&TextBold)
			alc32(ptr(x, sy - 1), wd, s, y2 - y,
				ptr(clipping.x1, sy - 1),
				ptr(clipping.x2, sy - 1),
				fore, (flags&TextItalic) != 0);
		if(flags&TextBold)
			alc32(ptr(x, sy - 1), wd, s, y2 - y,
				ptr(clipping.x1, sy - 1),
				ptr(clipping.x2, sy - 1),
				fore, (flags&TextItalic) != 0);
		alc32(ptr(x, sy), wd, s, y2 - y,
			ptr(clipping.x1, sy), ptr(clipping.x2, sy),
			fore, (flags&TextItalic) != 0);
		break;
	default:
		break;
	}
}

void draw::image(int x, int y, const sprite* e, int id, int flags, unsigned char alpha, color* pal) {
	auto pal_push = draw::palt;
	draw::palt = pal;
	image(x, y, e, id, flags | ImagePallette, alpha);
	draw::palt = pal_push;
}

void draw::stroke(int x, int y, const sprite* e, int id, int flags, unsigned char thin, unsigned char* koeff) {
	color tr;
	tr.a = 0;
	tr.r = 255;
	tr.g = 255;
	tr.b = 255;
	auto fr = e->get(id);
	rect rc = fr.getrect(x, y, flags);
	surface canvas(rc.width() + 2, rc.height() + 2, 32);
	x--; y--;
	if(true) {
		draw::state push;
		draw::canvas = &canvas;
		draw::setclip();
		draw::rectf({0, 0, canvas.width, canvas.height}, tr);
		draw::image(1, 1, e, id, ImageNoOffset);
	}
	for(int y1 = 0; y1 < canvas.height; y1++) {
		bool inside = false;
		for(int x1 = 0; x1 < canvas.width; x1++) {
			auto m = (color*)canvas.ptr(x1, y1);
			if(!inside) {
				if(*m == tr)
					continue;
				auto px = rc.x1 + x1 - 1;
				auto py = rc.y1 + y1 - 1;
				for(auto n = 0; n < thin; n++, px--) {
					if(koeff)
						draw::pixel(px, py, koeff[n]);
					else
						draw::pixel(px, py);
				}
				inside = true;
			} else {
				if(*m != tr)
					continue;
				auto px = rc.x1 + x1 - 2;
				auto py = rc.y1 + y1 - 1;
				for(auto n = 0; n < thin; n++, px++) {
					if(koeff)
						draw::pixel(px, py, koeff[n]);
					else
						draw::pixel(px, py);
				}
				inside = false;
			}
		}
	}
	for(int x1 = 0; x1 < canvas.width; x1++) {
		bool inside = false;
		for(int y1 = 0; y1 < canvas.height; y1++) {
			auto m = (color*)canvas.ptr(x1, y1);
			if(!inside) {
				if(*m == tr)
					continue;
				auto px = rc.x1 + x1 - 1;
				auto py = rc.y1 + y1 - 1;
				for(auto n = 0; n < thin; n++, py--) {
					if(koeff)
						draw::pixel(px, py, koeff[n]);
					else
						draw::pixel(px, py);
				}
				inside = true;
			} else {
				if(*m != tr)
					continue;
				auto px = rc.x1 + x1 - 1;
				auto py = rc.y1 + y1 - 2;
				for(auto n = 0; n < thin; n++, py++) {
					if(koeff)
						draw::pixel(px, py, koeff[n]);
					else
						draw::pixel(px, py);
				}
				inside = false;
			}
		}
	}
}

void draw::blit(surface& ds, int x1, int y1, int w, int h, unsigned flags, surface& ss, int xs, int ys) {
	if(ss.bpp != ds.bpp)
		return;
	int ox;
	if(!correctb(x1, y1, w, h, ox))
		return;
	if(flags&ImageTransparent)
		cpy32t(
			ds.ptr(x1, y1), ds.scanline,
			ss.ptr(xs, ys) + ox * 4, ss.scanline,
			w, h);
	else
		cpy(
			ds.ptr(x1, y1), ds.scanline,
			ss.ptr(xs, ys) + ox * 4, ss.scanline,
			w, h, 4);
}

void draw::blit(surface& dest, int x, int y, int width, int height, unsigned flags, surface& source, int x_source, int y_source, int width_source, int height_source) {
	if(width == width_source && height == height_source) {
		blit(dest, x, y, width, height, flags, source, x_source, y_source);
		return;
	}
	if(source.bpp != dest.bpp)
		return;
	if(!corrects(dest, x, y, width, height))
		return;
	if(!corrects(source, x_source, y_source, width_source, height_source))
		return;
	int ox;
	if(!correctb(x_source, y_source, width, height, ox))
		return;
	scale32(
		dest.ptr(x, y), dest.scanline, width, height,
		source.ptr(x_source, y_source) + ox * 4, source.scanline, width_source, height_source);
}

const pma* pma::getheader(const char* id) const {
	auto p = this;
	while(p->name[0]) {
		if(p->name[0] == id[0]
			&& p->name[1] == id[1]
			&& p->name[2] == id[2])
			return p;
		p = (pma*)((char*)p + p->size);
	}
	return 0;
}

const char* pma::getstring(int id) const {
	auto p = getheader("STR");
	if(!p || id > count)
		return "";
	return (char*)this + ((unsigned*)((char*)this + sizeof(*this)))[id];
}

int pma::find(const char* name) const {
	auto p = getheader("STR");
	for(int i = 1; i <= count; i++) {
		if(strcmp(getstring(i), name) == 0)
			return i;
	}
	return 0;
}

int sprite::ganim(int index, int tick) {
	if(!cicles)
		return 0;
	cicle* c = gcicle(index);
	if(!c->count)
		return 0;
	if(flags&NoIndex)
		return c->start + tick % c->count;
	return gindex(c->start + tick % c->count);
}

int sprite::glyph(unsigned sym) const {
	// First interval (latin plus number plus ASCII)
	unsigned* pi = (unsigned*)edata();
	unsigned* p2 = pi + esize() / sizeof(unsigned);
	unsigned n = 0;
	while(pi < p2) {
		if(sym >= pi[0] && sym <= pi[1])
			return sym - pi[0] + n;
		n += pi[1] - pi[0] + 1;
		pi += 2;
	}
	return 't' - 0x21; // Unknown symbol is question mark
}

rect sprite::frame::getrect(int x, int y, unsigned flags) const {
	int x2, y2;
	if(!offset)
		return{0, 0, 0, 0};
	if(flags&ImageMirrorH) {
		x2 = x;
		if((flags&ImageNoOffset) == 0)
			x2 += ox;
		x = x2 - sx;
	} else {
		if((flags&ImageNoOffset) == 0)
			x -= ox;
		x2 = x + sx;
	}
	if(flags&ImageMirrorV) {
		y2 = y;
		if((flags&ImageNoOffset) == 0)
			y2 += oy;
		y = y2 - sy;
	} else {
		if((flags&ImageNoOffset) == 0)
			y -= oy;
		y2 = y + sy;
	}
	return{x, y, x2, y2};
}

surface::plugin* surface::plugin::first;

surface::surface() : width(0), height(0), scanline(0), bpp(32), bits(0) {
}

surface::surface(int width, int height, int bpp) : surface() {
	resize(width, height, bpp, true);
}

surface::plugin::plugin(const char* name, const char* filter) : name(name), filter(filter), next(0) {
	seqlink(this);
}

unsigned char* surface::ptr(int x, int y) {
	return bits + y * scanline + x * (bpp / 8);
}

surface::~surface() {
	resize(0, 0, 0, true);
}

unsigned char* surface::allocator(unsigned char* bits, unsigned size) {
	if(!size) {
		free(bits);
		bits = 0;
	}
	if(!bits)
		bits = (unsigned char*)malloc(size);
	else
		bits = (unsigned char*)realloc(bits, size);
	return bits;
}

void surface::resize(int width, int height, int bpp, bool alloc_memory) {
	if(this->width == width && this->height == height && this->bpp == bpp)
		return;
	this->bpp = bpp;
	this->width = width;
	this->height = height;
	this->scanline = color::scanline(width, bpp);
	if(alloc_memory)
		bits = allocator(bits, (height + 1)*scanline);
}

void surface::flipv() {
	color::flipv(bits, scanline, height);
}

void surface::convert(int new_bpp, color* pallette) {
	if(bpp == new_bpp) {
		bpp = iabs(new_bpp);
		return;
	}
	auto old_scanline = scanline;
	scanline = color::scanline(width, new_bpp);
	if(iabs(new_bpp) <= bpp)
		color::convert(bits, width, height, new_bpp, 0, bits, bpp, pallette, old_scanline);
	else {
		unsigned char* new_bits = (unsigned char*)allocator(0, (height + 1)*scanline);
		color::convert(
			new_bits, width, height, new_bpp, pallette,
			bits, bpp, pallette, old_scanline);
		allocator(bits, 0);
		bits = new_bits;
	}
	bpp = iabs(new_bpp);
}

char* draw::key2str(char* result, int key) {
	result[0] = 0;
	if(key&Ctrl)
		zcat(result, "Ctrl+");
	if(key&Alt)
		zcat(result, "Alt+");
	if(key&Shift)
		zcat(result, "Shift+");
	key = key & 0xFFFF;
	switch(key) {
	case KeyDown: zcat(result, "Down"); break;
	case KeyDelete: zcat(result, "Del"); break;
	case KeyEnd: zcat(result, "End"); break;
	case KeyEnter: zcat(result, "Enter"); break;
	case KeyHome: zcat(result, "Home"); break;
	case KeyLeft: zcat(result, "Left"); break;
	case KeyPageDown: zcat(result, "Page Down"); break;
	case KeyPageUp: zcat(result, "Page Up"); break;
	case KeyRight: zcat(result, "Right"); break;
	case KeyUp: zcat(result, "Up"); break;
	case F1: zcat(result, "F1"); break;
	case F2: zcat(result, "F2"); break;
	case F3: zcat(result, "F3"); break;
	case F4: zcat(result, "F4"); break;
	case F5: zcat(result, "F5"); break;
	case F6: zcat(result, "F6"); break;
	case F7: zcat(result, "F7"); break;
	case F8: zcat(result, "F8"); break;
	case F9: zcat(result, "F9"); break;
	case F10: zcat(result, "F10"); break;
	case F11: zcat(result, "F11"); break;
	case F12: zcat(result, "F12"); break;
	case KeySpace: zcat(result, "Space"); break;
	default:
		zcat(result, char(szupper(key - Alpha)));
		break;
	}
	return result;
}