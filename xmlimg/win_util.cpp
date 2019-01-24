#include "xface/crt.h"
#include "xface/win.h"
#include "util.h"

using namespace util;

font::font(const char* name, int size) {
	wchar_t name1[260];
	wchar_t* d = name1;
	const char* s = name;
	while(*s)
		*d++ = *s++;
	*d++ = 0;
	void* hf = CreateFontW(size, 0, 0, 0, FW_NORMAL, 0, 0,
		0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FF_DONTCARE,
		name1);
	hcnv = CreateCompatibleDC(0);
	hfnt = SelectObject(hcnv, hf);
}

font::~font() {
	DeleteObject(SelectObject(hcnv, hfnt));
	DeleteDC(hcnv);
}

// tool used to create font with SubPixel rendering from system font's
int font::glyphi(int glyph,
	int& width, int& height, int& dx, int& dy, int& ox, int& oy,
	unsigned char* buffer, int maxsize, int xscale, int yscale) {
	MAT2 scale3h;
	memset(&scale3h, 0, sizeof(MAT2));
	scale3h.eM11.value = xscale; // rgb color
	scale3h.eM22.value = yscale;
	GLYPHMETRICS gm;
	memset(&gm, 0, sizeof(gm));
	int size = GetGlyphOutlineW(hcnv,
		glyph,
		GGO_GRAY8_BITMAP,
		&gm,
		maxsize,
		buffer,
		&scale3h);
	if(!size) {
		size = GetGlyphOutlineW(hcnv,
			glyph,
			GGO_GRAY8_BITMAP,
			&gm,
			size,
			buffer,
			&scale3h);
		if(!size)
			return false;
	}
	width = gm.gmBlackBoxX;
	height = gm.gmBlackBoxY;
	dx = gm.gmCellIncX;
	dy = gm.gmCellIncY;
	ox = gm.gmptGlyphOrigin.x;
	oy = gm.gmptGlyphOrigin.y;
	return size;
}

void font::info(int& width, int& height, int& dy1, int& dy2) {
	TEXTMETRICA tm;
	GetTextMetricsA(hcnv, &tm);
	width = tm.tmAveCharWidth;
	height = tm.tmHeight;
	dy1 = tm.tmAscent;
	dy2 = tm.tmDescent;
}