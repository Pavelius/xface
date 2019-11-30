#include "anyval.h"
#include "bsreq.h"
#include "crt.h"
#include "draw_control.h"
#include "markup.h"

using namespace draw;

const markup* getmarkup(const bsreq* type);

namespace {
enum title_s : unsigned char { TitleNormal, TitleShort, NoTitle };
struct commandi {
	void*				object;
	array*				data;
	const bsreq*		meta;
	markup::proci		proc;
	markup::propi		prop;
	rect				rc;
	anyval				value;
	void clear() { memset(this, 0, sizeof(commandi)); }
};
static commandi	command;
static void set_command_value() {
	command.value.set(hot.param);
}
struct contexti {
	int					title;
	const char*			title_text;
	title_s				title_state;
	int*				right;
	void*				object;
	const bsreq*		type;
	bool				show_missed_requisit;
	contexti() { memset(this, 0, sizeof(*this)); }
	bool isallow(const markup& e, int index) const {
		if(e.proc.isallow) {
			if(!e.proc.isallow(object, index))
				return false;
		}
		return true;
	}
};
}

static const char* getpresent(const void* p, const bsreq* type) {
	auto pf = type->find("name");
	if(!pf)
		pf = type->find("id");
	if(!pf)
		pf = type->find("text");
	if(!pf || !pf->is(KindText))
		return "";
	return (const char*)pf->get(pf->ptr(p));
}

static void choose_enum() {
	struct enum_view : controls::list, adat<int, 512> {
		markup::proci	proc;
		markup::propi	prop;
		const array&	source;
		const bsreq*	meta;
		const char*	getname(char* result, const char* result_max, int line, int column) const {
			if(prop.getname)
				return prop.getname(source.ptr(data[line]), result, result_max, 0);
			switch(column) {
			case 0: return getpresent(source.ptr(data[line]), meta);
			}
			return "";
		}
		int getmaximum() const override { return count; }
		int getcurrent() const {
			if(!count)
				return 0;
			return data[current];
		}
		constexpr enum_view(const bsreq* meta, const array& source) : source(source), proc(), meta(meta) {}
	};
	enum_view ev(command.meta, *command.data);
	ev.hilite_odd_lines = false;
	ev.proc = command.proc;
	auto i1 = 0;
	auto i2 = command.data->getcount() - 1;
	for(unsigned i = i1; i <= i2; i++) {
		if(command.proc.isallow) {
			if(!command.proc.isallow(command.object, i))
				continue;
		}
		ev.add(i);
	}
	auto mc = imin(12, ev.getmaximum());
	auto rc = command.rc;
	auto ci = ev.indexof(command.value.get());
	if(ci != -1)
		ev.current = ci;
	rc.y1 = rc.y2;
	rc.y2 = rc.y1 + ev.getrowheight() * mc + 1;
	rectf(rc, colors::form);
	if(dropdown(rc, ev))
		command.value.set(ev.getcurrent());
}

static bsval getvalue(contexti& ctx, const markup& e) {
	bsval result;
	result.type = ctx.type->find(e.value.id);
	result.data = ctx.object;
	return result;
}

static rect start_group(int& x, int& y, int& width) {
	setposition(x, y, width);
	rect rc = {x, y, x + width, y + texth() + 4 * 2};
	y += rc.height();
	setposition(x, y, width);
	return rc;
}

static int close_group(int x, int y, const rect& rc, const char* title) {
	if(!rc)
		return 0;
	if(title) {
		gradv(rc, colors::border, colors::edit);
		text(rc, title, AlignCenterCenter);
		rectb({rc.x1, rc.y1, rc.x2, y + metrics::padding}, colors::border);
		line(rc.x1, rc.y2, rc.x2, rc.y2, colors::border);
	}
	return metrics::padding * 2;
}

static int element(int x, int y, int width, contexti& ctx, const markup& e);

static int group_vertial(int x, int y, int width, contexti& ctx_original, const markup* elements) {
	if(!elements)
		return 0;
	auto y0 = y;
	auto ctx = ctx_original;
	for(auto p = elements; *p; p++)
		y += element(x, y, width, ctx, *p);
	return y - y0;
}

static int group_horizontal(int x, int y, int width, contexti& ctx_original, const markup* elements) {
	if(!elements)
		return 0;
	auto ym = 0;
	auto wi = width / 12;
	auto wp = 0;
	auto ctx = ctx_original;
	for(auto p = elements; *p; p++) {
		auto x1 = x + wp * wi;
		auto we = p->width*wi;
		if(x1 + we > width - 12)
			we = width - x1;
		auto yc = element(x, y, we, ctx, *p);
		if(ym < yc)
			ym = yc;
		x += we;
	}
	return ym;
}

static void header(int& x, int y, int& width, contexti& ctx, const char* label) {
	if(ctx.title_text && ctx.title_text[0]) {
		label = ctx.title_text;
		ctx.title_text = 0;
	}
	if(!label || !label[0])
		return;
	if(ctx.title_state == NoTitle)
		return;
	if(ctx.title_state == TitleShort) {
		auto w = textw(label);
		titletext(x, y, width, 0, label, w);
		x += metrics::padding;
		width -= metrics::padding;
	} else
		titletext(x, y, width, 0, label, ctx.title);
}

static int error(int x, int y, int width, contexti& ctx, const markup& e, const char* error_text) {
	auto old_color = fore;
	fore = colors::red;
	setposition(x, y, width);
	titletext(x, y, width, 0, e.title ? e.title : e.value.id, ctx.title, ":");
	auto h = texth() + 4 * 2;
	rect rc = {x, y, x + width, y + h};
	rectb(rc, colors::red);
	text(rc, error_text, AlignCenterCenter);
	fore = old_color;
	return rc.height() + metrics::padding * 2;
}

//void field_enum(const rect& rc, unsigned flags, const anyval& ev, const array* meta_type, const void* object, const markup::proci* pri, const markup::propi* ppi) {
//	auto pb = bsdata::find(meta_type);
//	if(!pb)
//		return;
//	char temp[128];
//	auto focused = isfocused(flags);
//	auto result = focused && (hot.key == KeyEnter || hot.key == F2);
//	if(buttonh(rc, false, focused, false, true, 0, 0, false, 0))
//		result = true;
//	auto pn = "";
//	auto pv = pb->get(ev.get());
//	if(ppi && ppi->getname)
//		pn = ppi->getname(pv, temp, zendof(temp), meta_type);
//	else
//		pn = getpresent(pb->get(ev.get()), pb->meta);
//	textc(rc.x1 + 4, rc.y1 + 4, rc.width() - 4 * 2, pn);
//	if(focused)
//		rectx({rc.x1 + 2, rc.y1 + 2, rc.x2 - 2, rc.y2 - 2}, colors::border);
//	if(result) {
//		command.clear();
//		command.rc = rc;
//		command.value = ev;
//		command.data = pb;
//		command.object = (void*)object;
//		if(pri) {
//			//if(pri->command) {
//			//	cmd(pri->command, (void*)pv).execute();
//			//	return;
//			//}
//			command.proc = *pri;
//			command.prop = *ppi;
//		}
//		execute(choose_enum);
//	}
//}

static int field_main(int x, int y, int width, contexti& ctx, const char* title_text, void* pv, const bsreq* type, int param, const markup* child, const markup::proci* pri, const markup::propi* ppi) {
	auto xe = x + width;
	auto y0 = y;
	setposition(x, y, width);
	header(x, y, width, ctx, title_text);
	rect rc = {x, y, x + width, y + draw::texth() + 8};
	unsigned flags = AlignLeft;
	if(type->is(KindNumber))
		flags = AlignRight;
	anyval av(pv, type->size, 0);
	if(draw::isfocused(rc, av))
		flags |= Focused;
	if(type->is(KindText))
		draw::field(rc, flags, av, -1, false, true, 0);
	else if(type->is(KindEnum) || (type->is(KindNumber) && type->source)) {
		//auto hint = type->type;
		//if(type->hint_type)
		//	hint = type->hint_type;
		//field_enum(rc, flags, av, hint, ctx.source.data, pri, ppi);
	} else if(type->is(KindNumber)) {
		auto d = param;
		if(!d)
			d = 2;
		if(d != -1) {
			auto wn = draw::textw("0");
			auto we = wn * (d + 1) + (draw::texth() + 8) + 4 * 2;
			rc.x2 = rc.x1 + we;
		}
		draw::field(rc, flags, av, d, true, false, 0);
		if(ctx.right)
			*ctx.right = rc.x2;
		if(child) {
			auto ctx1 = ctx;
			ctx1.right = &rc.x2;
			ctx1.title_state = TitleShort;
			for(auto p = child; *p && rc.x2 < xe; p++)
				element(rc.x2, y0, xe - rc.x2, ctx1, *p);
		}
	}
	return rc.height() + metrics::padding*2;
}

static int element(int x, int y, int width, contexti& ctx, const markup& e) {
	if(e.proc.isvisible && !e.proc.isvisible(ctx.object, e.value.index))
		return 0;
	else if(e.value.id && e.value.id[0] == '#')
		// Страницы, команды, любые другие управляющие структуры.
		return 0;
	else if(e.proc.custom) {
		auto pv = ctx.object;
		if(e.value.id) {
			auto bv = getvalue(ctx, e);
			if(bv.type)
				pv = bv.type->ptr(bv.data, e.value.index);
		}
		if(e.title) {
			auto dy = e.proc.custom(x + ctx.title, y, width - ctx.title, pv);
			if(dy) {
				setposition(x, y, width);
				header(x, y, width, ctx, e.title);
			}
			return dy;
		}
		return e.proc.custom(x, y, width, pv);
	} else if(e.cmd.execute) {
		auto result = false;
		//auto dy = button(x, y, width, (int)ctx.source.data, result, e.title);
		//if(result) {
		//	//
		//}
		//return dy;
		return 0;
	}
	else if(e.title && e.title[0] == '#') {
		auto pn = e.title + 1;
		auto y0 = y;
		if(strcmp(pn, "title") == 0) {
			ctx.title = e.param;
			return 0;
		}
		if(!e.value.id)
			return 0;
		// Теперь специальные элементы с заполненными данными
		auto bv = getvalue(ctx, e);
		if(!bv.data || !bv.type)
			return 0;
		if(strcmp(pn, "checkboxes") == 0 && (bv.type->is(KindEnum) || bv.type->is(KindCFlags))) {
			auto pb = bv.type->source;
			if(!pb)
				return error(x, y, width, ctx, e, "Не найдена база");
			if(pb->getcount() == 0)
				return 0;
			auto size = bv.type->size;
			if(bv.type->is(KindCFlags))
				size = bv.type->lenght;
			auto columns = e.param;
			if(!columns)
				columns = 1;
			auto wc = width / columns;
			auto wi = 0;
			auto x0 = x;
			auto y1 = y;
			for(unsigned i = 0; i < pb->getcount(); i++) {
				if(!ctx.isallow(e, i))
					continue;
				auto p = getpresent(pb->ptr(i), bv.type->type);
				auto av = anyval(bv.type->ptr(bv.data), size, i);
				auto y0 = y + checkbox(x, y, wc, av, p, 0) + 2;
				if(y1 < y0)
					y1 = y0;
				if(++wi >= columns) {
					x = x0;
					y = y0;
					wi = 0;
				} else
					x += wc;
			}
			y = y1;
		} else if(strcmp(pn, "radiobuttons") == 0 && bv.type->is(KindEnum)) {
			auto pb = bv.type->source;
			if(!pb)
				return error(x, y, width, ctx, e, "Не найдена база");
			if(pb->getcount() == 0)
				return 0;
			auto size = bv.type->size;
			for(unsigned i = 0; i < pb->getcount(); i++) {
				if(!ctx.isallow(e, i))
					continue;
				auto p = getpresent(pb->ptr(i), bv.type->type);
				auto av = anyval(bv.type->ptr(bv.data), size, i);
				y += radio(x, y, width, av, p, 0) + 2;
			}
		} else {
			if(!bv.type->is(KindNumber))
				return error(x, y, width, ctx, e, bv.type->id);
			auto pb = bv.type->source;
			if(!pb)
				return error(x, y, width, ctx, e, pn);
			auto count = pb->getcount();
			if(count)
				return 0;
			if(count > bv.type->count)
				count = bv.type->count;
			for(unsigned i = 0; i < count; i++) {
				auto pv = bv.type->ptr(bv.data, i);
				if(!ctx.isallow(e, i))
					continue;
				y += field_main(x, y, width, ctx, getpresent(pb->ptr(i), bv.type->type), pv, bv.type, e.param, 0, &e.proc, &e.prop);
			}
		}
		return y - y0;
	} else if(e.value.id) {
		auto bv = getvalue(ctx, e);
		if(!bv.data || !bv.type) {
			if(ctx.show_missed_requisit)
				return error(x, y, width, ctx, e, "Не найден реквизит");
			return 0; // Данные не найдены, но это не ошибка
		}
		auto pv = bv.type->ptr(bv.data, e.value.index);
		// Вначале найдем целую форму объекта
		if(bv.type->is(KindScalar)) {
			auto hint_type = bv.type->type;
			auto pm = e.value.child;
			if(!pm)
				pm = getmarkup(hint_type);
			if(!pm)
				return error(x, y, width, ctx, e, "Не найдена разметка");
			auto ctx1 = ctx;
			ctx1.title_text = e.title;
			ctx1.object = pv;
			ctx1.type = hint_type;
			return group_vertial(x, y, width, ctx1, pm);
		} else
			return field_main(x, y, width, ctx, e.title, pv, bv.type, e.param, e.value.child, &e.proc, &e.prop);
	} else if(e.value.child) {
		rect rgo = {};
		auto y0 = y;
		if(e.title)
			rgo = start_group(x, y, width);
		auto y1 = y;
		if(e.value.child[0].width)
			y += group_horizontal(x, y, width, ctx, e.value.child);
		else
			y += group_vertial(x, y, width, ctx, e.value.child);
		if(y == y1)
			return 0;
		y += close_group(x, y, rgo, e.title);
		return y - y0;
	} else
		return 0;
}

int draw::field(int x, int y, int width, const markup* elements, const bsreq* type, void* object, int title_width) {
	if(!elements)
		return 0;
	contexti ctx;
	ctx.title = title_width;
	ctx.object = object;
	ctx.type = type;
	ctx.show_missed_requisit = true;
	if(elements->width)
		return group_horizontal(x, y, width, ctx, elements);
	else
		return group_vertial(x, y, width, ctx, elements);
}