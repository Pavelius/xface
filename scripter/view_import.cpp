#include "io.h"
#include "main.h"
#include "draw_control.h"

using namespace code;
using namespace draw;
using namespace draw::controls;

static void import_modules(vector<const char*>& result, const char* url) {
	char temp[260];
	for(auto e = io::file::find(url); e; e.next()) {
		auto p = e.name();
		if(p[0] == '.')
			continue;
		auto pe = szext(p);
		if(!pe) {
			e.fullname(temp);
			import_modules(result, temp);
		} else if(strcmp(pe, "mtd")==0) {
			auto len = zlen(metadata::classes_url);
			stringbuilder sb(temp);
			sb.add(url + len + 1);
			sb.add(".");
			sb.add(e.name());
			sb.change('/', '.');
			sb.change('\\', '.');
			temp[zlen(temp) - 4] = 0;
			result.add(szdup(temp));
		}
	}
}

class import_view : public control {
	class cinputview : public textedit {
		char				input[260];
		import_view*		parent;
	public:
		const char*			get() const { return input; }
		cinputview(import_view*	parent) : textedit(input, sizeof(input), false), parent(parent) {
			input[0] = 0;
		}
		bool keyinput(unsigned id) override {
			switch(id) {
			case InputSymbol:
				textedit::keyinput(id);
				parent->update();
				break;
			default:
				return textedit::keyinput(id);
			}
			return true;
		}
	};
	vector<const char*>		modules;
	tableref				cmodules;
	cinputview				cinput;
	void view(const rect& rco) {
		rect rc = rco;
		auto dy = texth() - cinput.rctext.height();
		cinput.view({rc.x1, rc.y1, rc.x2, rc.y1 + dy});
		rc.y1 += dy + metrics::padding;
		cmodules.view(rc);
	}
	bool isfocusable() const override { return false; }
public:
	static bool match(const char* text, const char* pattern) {
		if(!pattern[0])
			return true;
		while(*text) {
			for(int i = 0; stringbuilder::upper(text[i]) == stringbuilder::upper(pattern[i]); i++) {
				if(pattern[i + 1] == 0)
					return true;
			}
			text++;
		}
		return false;
	}
	void update() {
		cmodules.clear();
		for(auto& p : modules) {
			if(!match(p, cinput.get()))
				continue;
			cmodules.addref(&p);
		}
		//modulesv.sort(0, true);
	}
	import_view() : cinput(this) {
		cmodules.select_mode = SelectRow;
		cmodules.show_header = false;
		cmodules.read_only = true;
		cmodules.addcol("Модуль", {0, sizeof(const char*)}, "text");
		import_modules(modules, metadata::classes_url);
		update();
	}
};

void project::import() {
	import_view view;
	if(!controls::edit(view))
		return;
}