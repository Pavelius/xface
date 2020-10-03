#include "main.h"

using namespace code;

project project::main;

static void writer(serializer& file, const project& e) {
	file.open("Project");
	file.set("Name", e.name);
	file.set("Description", e.name);
	file.open("Modules", serializer::Array);
	for(auto& v : e.modules)
		file.set("", v);
	file.close("Modules", serializer::Array);
	file.close("Project");
}

void project::write(const char* url) {
}