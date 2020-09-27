#include "wizard.h"
#include "draw_control.h"

using namespace draw;

namespace {
enum command_s : unsigned char {
	NoCommand, Next, Previous, Finish
};
}

static command_s current_command;

const wizard::element* wizard::getvalid(const wizard::element* p) const {
	auto pc = const_cast<wizard*>(this);
	while(p && *p) {
		if((pc->*p->proc.call)({},Try))
			return p;
		p++;
	}
	return 0;
}

const wizard::element* wizard::getlast(const wizard::element* p) const {
	if(!p)
		return false;
	auto pc = const_cast<wizard*>(this);
	const element* p1 = 0;
	while(*p) {
		if((pc->*p->proc.call)({}, Try))
			p1 = p;
		p++;
	}
	return p1;
}

void wizard::choosefolder(const anyval& value) {
	char temp[260] = {};
	if(!dialog::folder("Выбирайте папку", temp))
		return;
	auto p = (char*)value.getptr();
	auto n = zlen(temp);
	memcpy(p, temp, n);
	p[n] = 0;
}

static void next_step() {
	current_command = Next;
}

static void finish_step() {
	current_command = Finish;
}

static void prev_step() {
	current_command = Previous;
}

static int buttonw(int x, int y, const char* text, fnevent proc, int key) {
	auto w = textw(text) + 8;
	if(w < 100)
		w = 100;
	button(x - w, y, w, proc, text, 0, key);
	return w;
}

bool wizard::show(const char* title) {
	auto p_start = getelements();
	auto pc = getvalid(p_start);
	if(pc)
		(this->*pc->proc.call)({}, Initialize);
	while(ismodal()) {
		if(!pc)
			break;
		rect rc = {0, 0, getwidth(), getheight()};
		rectf(rc, colors::form);
		if(title) {
			state push;
			auto c2 = colors::text.mix(colors::edit);
			auto c1 = colors::border;
			font = metrics::h2;
			rect r1 = rc; r1.y2 = r1.y1 + texth() + metrics::padding*2;
			gradv(r1, c1, c2);
			line(r1.x1, r1.y2, r1.x2, r1.y2, colors::border);
			rc.y1 += r1.height() + metrics::padding * 4;
			r1.offset(metrics::padding * 2, metrics::padding);
			text(r1, title, AlignLeft);
		}
		auto bottom_height = texth() + 8 + metrics::padding * 2;
		rect r2 = rc;
		r2.offset(32 + metrics::padding, 0);
		if(pc->title) {
			r2.y1 += text(r2, pc->title, AlignLeft);
			r2.offset(metrics::padding * 2, metrics::padding * 2);
		}
		if(pc->proc.call) {
			rect r1 = r2;
			r1.y1 += metrics::padding * 2;
			r1.y2 -= metrics::padding * 2;
			r1.y2 -= bottom_height - 1;
			(this->*pc->proc.call)(r1, Draw);
		}
		if(true) {
			rc.y1 = rc.y2 - bottom_height - 1;
			line(rc.x1, rc.y1 + 1, rc.x2, rc.y1 + 1, colors::border);
			auto y = rc.y1 + metrics::padding;
			rc.x2 -= buttonw(rc.x2, y, "Отмена", buttoncancel, KeyEscape);
			if(getlast(pc)!=pc)
				rc.x2 -= buttonw(rc.x2, y, "Далее", next_step, 0);
			else
				rc.x2 -= buttonw(rc.x2, y, "Готово", finish_step, KeyEnter);
			if(p_start != pc)
				rc.x2 -= buttonw(rc.x2, y, "Назад", prev_step, 0);
		}
		current_command = NoCommand;
		domodal();
		switch(current_command) {
		case Previous: break;
		case Next:
			pc = getvalid(pc + 1);
			if(pc)
				(this->*pc->proc.call)({}, Initialize);
			break;
		case Finish: breakmodal(1); break;
		default: break;
		}
	}
	return getresult() != 0;
}