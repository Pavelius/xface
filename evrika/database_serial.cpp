#include "xface/io.h"
#include "main.h"

bool database::readfile(const char* url) {
	io::file file(url, StreamRead);
	if(!file)
		return false;
	return true;
}