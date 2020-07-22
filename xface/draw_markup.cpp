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
	markup::cmdi		prod;
	rect				rc;
	anyval				value;
	void clear() { memset(this, 0, sizeof(commandi)); }
};
static commandi	command;
struct contexti {
	int					title;
	const char*			title_text;
	title_s				title_state;
	int*				right;
	void*				object;
	const bsreq*		type;
	bool				show_missed_requisit;
	controls::form*		form;
	int					y2;
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

static const bsreq* getvalue(contexti& ctx, const markup& e) {
	return ctx.type->find(e.value.id);
}

static controls::control* getcontrol(contexti& ctx, const markup& e) {
	return 0;
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

static int group_vertial(int x, int y, int width, const contexti& ctx_original, const markup* elements) {
	if(!elements)
		return 0;
	auto y0 = y;
	auto ctx = ctx_original;
	for(auto p = elements; *p; p++)
		y += element(x, y, width, ctx, *p);
	return y - y0;
}

static int group_horizontal(int x, int y, int width, const contexti& ctx_original, const markup* elements) {
	if(!elements)
		return 0;
	auto ym = 0;
	auto wi = width / 12;
	auto wp = 0;
	auto ctx = ctx_original;
	auto x2 = x + width;
	for(auto p = elements; *p; p++) {
		auto x1 = x + wp * wi;
		auto we = p->width*wi;
		if(x1 + we > x2 - 12)
			we = x2 - x1;
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

static int field_main(int x, int y, int width, contexti& ctx, const char* title_text, void* pv, const bsreq* type, int param, const markup* child, const markup::proci* pri, const markup::propi* ppi) {
	auto xe = x + width;
	auto y0 = y;
	setposition(x, y, width);
	header(x, y, width, ctx, title_text);
	rect rc = {x, y, x + width, y + draw::texth() - metrics::edit.height()};
	unsigned flags = AlignLeft;
	if(type->is(KindNumber))
		flags = AlignRight;
	anyval av(pv, type->size, 0);
	if(draw::isfocused(rc, av))
		flags |= Focused;
	if(type->is(KindText))
		fieldf(rc, flags, av, -1, false, true, 0);
	else if(type->is(KindEnum) || (type->is(KindNumber) && type->source)) {
		if(type->source)
			fieldc(rc, av, *type->source, ppi->getname, 0, ctx.object, pri->isallow);
	} else if(type->is(KindNumber)) {
		auto d = param;
		if(!d)
			d = 2;
		if(d != -1) {
			auto wn = draw::textw("0");
			auto we = wn * (d + 1) + (draw::texth() + 8) + 4 * 2;
			rc.x2 = rc.x1 + we;
		}
		fieldf(rc, flags, av, d, true, false, 0);
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
	return rc.height() - metrics::edit.height();
}

static void execute_command() {
	command.prod.execute(command.object);
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
			auto type = getvalue(ctx, e);
			if(type)
				pv = type->ptr(ctx.object, e.value.index);
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
		setposition(x, y, width);
		rect rc = {x, y, x + width, y + texth() - metrics::edit.height()};
		anyval av(e.cmd.execute, sizeof(e), (int)ctx.object);
		unsigned flags = 0;
		auto focused = false;
		if(isfocused(rc, av))
			focused = true;
		auto result = buttonh(rc, false, focused, false, true, e.title);
		if(result) {
			command.clear();
			command.object = ctx.object;
			command.prod = e.cmd;
			execute(execute_command);
		}
		return rc.height() + metrics::padding*2;
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
		auto req = getvalue(ctx, e);
		if(!req)
			return 0;
		if(strcmp(pn, "checkboxes") == 0 && (req->is(KindEnum) || req->is(KindCFlags))) {
			auto pb = req->source;
			if(!pb)
				return error(x, y, width, ctx, e, "Не найдена база");
			if(pb->getcount() == 0)
				return 0;
			auto size = req->size;
			if(req->is(KindCFlags))
				size = req->lenght;
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
				auto p = getpresent(pb->ptr(i), req->type);
				auto av = anyval(req->ptr(ctx.object), size, i);
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
		} else if(strcmp(pn, "radiobuttons") == 0 && req->is(KindEnum)) {
			auto pb = req->source;
			if(!pb)
				return error(x, y, width, ctx, e, "Не найдена база");
			if(pb->getcount() == 0)
				return 0;
			auto size = req->size;
			for(unsigned i = 0; i < pb->getcount(); i++) {
				if(!ctx.isallow(e, i))
					continue;
				auto p = getpresent(pb->ptr(i), req->type);
				auto av = anyval(req->ptr(ctx.object), size, i);
				y += radio(x, y, width, av, p, 0) + 2;
			}
		} else if(strcmp(pn, "tabs") == 0 && req->is(KindNumber)) {
			if(!e.value.child)
				return 0;
			const markup* pages[32];
			auto pb = pages;
			auto pe = pages + sizeof(pages)/ sizeof(pages[0]);
			for(auto p = e.value.child; *p; p++) {
				if(p->proc.isvisible && !p->proc.isvisible(ctx.object, 0))
					continue;
				if(pb<pe)
					*pb++ = p;
			}
			auto count = pb - pages;
			if(!count)
				return 0;
			auto av = anyval(req->ptr(ctx.object, e.value.index), req->size, 0);
			auto active = av.get();
			int result_active;
			rect rc = {x, y, x + width, y + texth() + 4 * 2};
			auto result = tabs(rc, false, false, (void**)pages, 0, count, active, &result_active, markup::getname, {});
			y += rc.height();
			element(x, y, width, ctx, *pages[active]);
		} else {
			if(!req->is(KindNumber))
				return error(x, y, width, ctx, e, req->id);
			auto pb = req->source;
			if(!pb)
				return error(x, y, width, ctx, e, pn);
			auto count = pb->getcount();
			if(!count)
				return 0;
			if(count > req->count)
				count = req->count;
			for(unsigned i = 0; i < count; i++) {
				auto pv = req->ptr(ctx.object, i);
				if(!ctx.isallow(e, i))
					continue;
				y += field_main(x, y, width, ctx, getpresent(pb->ptr(i), req->type), pv, req, e.param, 0, &e.proc, &e.prop);
			}
		}
		return y - y0;
	} else if(e.value.id) {
		auto req = getvalue(ctx, e);
		if(!req) {
			if(ctx.form) {
				auto pc = ctx.form->getcontrol(e.value.id);
				if(pc) {
					setposition(x, y, width);
					if(pc->show_toolbar)
						y += pc->toolbar(x, y, width);
					auto splitter = pc->splitter;
					if(!splitter && ctx.y2)
						splitter = ctx.y2 - y - metrics::padding;
					if(!splitter)
						splitter = 100;
					rect rc = {x, y, x + width, y + splitter};
					pc->view(rc);
					return rc.height();
				}
			}
			if(ctx.show_missed_requisit)
				return error(x, y, width, ctx, e, "Не найден реквизит");
			return 0; // Данные не найдены, но это не ошибка
		}
		auto pv = req->ptr(ctx.object, e.value.index);
		// Вначале найдем целую форму объекта
		if(req->is(KindScalar)) {
			auto hint_type = req->type;
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
			return field_main(x, y, width, ctx, e.title, pv, req, e.param, e.value.child, &e.proc, &e.prop);
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

int draw::field(int x, int y, int width, const markup* elements, const bsreq* type, void* object, int title_width, controls::form* form, int height) {
	if(!elements)
		return 0;
	contexti ctx;
	ctx.title = title_width;
	ctx.object = object;
	ctx.type = type;
	ctx.form = form;
	ctx.show_missed_requisit = true;
	if(height)
		ctx.y2 = y + height;
	if(elements->width)
		return group_horizontal(x, y, width, ctx, elements);
	else
		return group_vertial(x, y, width, ctx, elements);
}