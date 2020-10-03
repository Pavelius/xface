#include "io.h"
#include "main.h"

using namespace code;

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

void project::import() {
	vector<const char*> result;
	import_modules(result, metadata::classes_url);
}