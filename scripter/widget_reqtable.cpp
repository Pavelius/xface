#include "main.h"
#include "draw_control.h"

using namespace code;
using namespace draw;

class widget_reqtable : public controls::table {
	const requisit*		source;
	array				source_array;
public:
	widget_reqtable(const requisit* v) : source(v), source_array(v->getlenght()) {
		auto parent_type = v->type;
		if(!parent_type)
			return;
		for(auto& e : bsdata<requisit>()) {
			if(!e)
				continue;
			if(e.parent != parent_type)
				continue;
			if(e.is(Static))
				continue;
			addcol(e.id, {(unsigned short)e.offset, (unsigned char)e.getsize(), 0}, "text");
		}
	}
};

static bool istag(const char* p, const char* pv, const char** pr = 0) {
	while(*pv) {
		if(*p++ != *pv++)
			return false;
	}
	if(pr)
		*pr = p;
	return true;
}

static const char* skiplit(const char* p, const char* symbols) {
	while(ischa(*p) || isnum(*p) || zchr(symbols, *p))
		p++;
	return p;
}

static const metadata* parsemeta(const char* p, const char** pr) {
	if(istag(p, "req://", &p)) {
		auto p2 = skiplit(p, "._");
		auto n = p2 - p;
		for(auto& e : bsdata<metadata>()) {
			if(!e)
				continue;
			if(memcmp(e.id, p, n) == 0) {
				if(pr)
					*pr = p2;
				return &e;
			}
		}
	}
	return 0;
}

static const requisit* parsereq(const char* p, const metadata* type, const char** pr) {
	auto p2 = skiplit(p, "._");
	auto n = p2 - p;
	for(auto& e : bsdata<requisit>()) {
		if(!e)
			continue;
		if(e.type != type)
			continue;
		if(memcmp(e.id, p, n) == 0) {
			if(pr)
				*pr = p2;
			return &e;
		}
	}
	return 0;
}

static class plugin_reqtable : controls::control::plugin, controls::control::plugin::builder {
	controls::control* create(const char* url) override {
		auto p = url;
		auto type = parsemeta(p, &p);
		if(!type)
			return 0;
		auto req = parsereq(p, type, &p);
		if(!req)
			return 0;
		return new widget_reqtable(req);
	}
	bool canopen(const char* url) const override {
		return szstart(url, "req://");
	}
	void destroy(controls::control* p) override {
	}
	controls::control::plugin::builder* getbuilder() override {
		return this;
	}
	controls::control* getcontrol() override {
		return 0;
	}
public:
	plugin_reqtable() : plugin("reqtable", DockWorkspace) {
	}
} plugin_control;