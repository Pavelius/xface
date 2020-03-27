#include "crt.h"
#include "draw.h"

using namespace draw;

int draw::sheetline(rect rc, bool background) {
	if(background)
		gradv(rc, colors::border.lighten(), colors::border.darken());
	rectf({rc.x1, rc.y2 - 2, rc.x2, rc.y2}, colors::active);
	return rc.height();
}

static bool sheet(rect& rc, rect& rct, const char* string, areas* area_result, bool checked, bool right_side, int w2) {
	bool result = false;
	int width = textw(string) + w2;
	if(right_side)
		rct.set(rc.x2 - width - 8, rc.y1, rc.x2, rc.y2);
	else
		rct.set(rc.x1, rc.y1, rc.x1 + width + 8, rc.y2);
	auto a = area(rct);
	if(area_result)
		*area_result = a;
	if(checked) {
		gradv({rct.x1 + 1, rct.y1 + 1, rct.x2, rct.y2 - 2},
			colors::tabs::back, colors::active);
		rectb({rct.x1 + 1, rct.y1 + 1, rct.x2, rct.y2 - 1},
			colors::active);
	}
	text({rct.x1, rct.y1, rct.x2 - w2, rct.y2}, string, AlignCenterCenter);
	if(a != AreaNormal) {
		if(a == AreaHilitedPressed) {
			if(hot.key == MouseLeft)
				result = true;
		} else if(a == AreaHilited) {
			draw::rectf({rct.x1 + 1, rct.y1 + 1, rct.x2, rct.y2 - 2},
				colors::tabs::back, 64);
		}
	}
	if(right_side)
		rc.x2 -= rct.width();
	else
		rc.x1 += rct.width();
	return result;
}

int draw::tabs(rect rc, bool show_close, bool right_side, void** data, int start, int count, int current, int* hilite,
	fntext gtext, rect position) {
	draw::state push;
	//rect rco = rc + position;
	setclip(rc);
	char temp[260];
	auto result = 0;
	// По-умолчанию нет подсвеченного элемента
	if(hilite)
		*hilite = -1;
	// Получим выравнивание
	//int ox1 = rc.x1;
	for(int i = start; i < count; i++) {
		if(rc.x1 >= rc.x2 || rc.x2 <= rc.x1)
			break;
		auto object = data[i]; temp[0] = 0;
		auto s = gtext(object, temp, temp + sizeof(temp)-1, 0);
		if(i == current)
			fore = colors::tabs::text;
		else
			fore = colors::text;
		areas a; rect element;
		if(sheet(rc, element, s, &a, (i == current), right_side, (show_close ? 16 : 0)))
			result = 1;
		if(a == AreaHilited || a == AreaHilitedPressed) {
			if(hilite)
				*hilite = i;
		}
		if((a == AreaHilited || a == AreaHilitedPressed || (i == current)) && show_close) {
			const int dy = 12;
			rect rcx;
			rcx.y1 = element.y1 + (element.height() - dy) / 2;
			rcx.y2 = rcx.y1 + dy;
			rcx.x1 = element.x2 - dy - 4;
			rcx.x2 = rcx.x1 + rcx.height();
			if(areb(rcx)) {
				if(buttonh(rcx, false, false, false, true, 0))
					result = 2;
				tooltips("Закрыть");
			}
			line(rcx.x1 + 4, rcx.y1 + 4, rcx.x2 - 4, rcx.y2 - 4, fore);
			line(rcx.x2 - 4, rcx.y1 + 4, rcx.x1 + 4, rcx.y2 - 4, fore);
		}
	}
	return result;
}

int draw::tabs(int x, int y, int width, bool show_close, bool right_side, void** data, int start, int count, int current, int* hilite,
	fntext gtext, rect position) {
	return tabs({x, y, x + width, y + draw::texth() + metrics::padding * 4},
		show_close, right_side, data, start, count, current, hilite, gtext, position);
}