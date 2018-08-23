#include "draw.h"

inline short* fwidth(const sprite* font) {
	return (short*)((char*)font + font->size - font->count * sizeof(short));
}

inline int wsymbol(const sprite* font, unsigned u) {
	return (u <= 0x20) ? 't' - 0x21 : font->glyph(u);
}

int draw::textw(int sym) {
	if(!font)
		return 0;
	return fwidth(font)[wsymbol(font, sym)];
}

int draw::textw(sprite* font) {
	if(!font)
		return 0;
	return fwidth(font)[wsymbol(font, 'A')];
}

void draw::glyph(int x, int y, int sym, unsigned flags) {
	static unsigned char koeff[] = {128, 160};
	int id = font->glyph(sym);
	if(sym >= 0x21) {
		if(flags&TextStroke) {
			color push_fore = fore;
			fore = fore_stroke;
			stroke(x, y + font->ascend, font, id, flags, 2, koeff);
			fore = push_fore;
		}
		image(x, y + font->ascend, font, id, flags, 0);
	}
}

int draw::texth() {
	if(!font)
		return 0;
	return font->height;
}