#include "io.h"
#include "fileimport.h"
#include "stringbuilder.h"

const char* fileimport::geturl(char* buffer) const {
	stringbuilder sb(buffer, buffer + 259);
	sb.add("default/%1.dat", name);
	return buffer;
}

fileimport::fileimport(const char* name, void* data, unsigned size) : name(name), data(data), size(size), need_serial(true) {
	if(!size || !data)
		return;
	char temp[260];
	io::file file(geturl(temp), StreamRead);
	if(!file)
		return;
	file.read(data, size);
}

fileimport::~fileimport() {
	if(!size || !data || !need_serial)
		return;
	char temp[260];
	io::file file(geturl(temp), StreamWrite);
	file.write(data, size);
}