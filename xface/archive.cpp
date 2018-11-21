#include "archive.h"

bool archive::signature(const char* id) {
	char temp[4];
	if(writemode) {
		memset(temp, 0, sizeof(temp));
		zcpy(temp, id, sizeof(temp) - 1);
		source.write(temp, sizeof(temp));
	} else {
		source.read(temp, sizeof(temp));
		if(szcmpi(temp, id) != 0)
			return false;
	}
	return true;
}

bool archive::version(short major, short minor) {
	short major_reader = major;
	short minor_reader = minor;
	set(major_reader);
	set(minor_reader);
	if(!writemode) {
		if(major_reader < major)
			return false;
		else if(major_reader == major && minor_reader < minor)
			return false;
	}
	return true;
}

void archive::setpointer(void** value) {
	unsigned pid;
	if(writemode) {
		pid = -1;
		auto j = 0;
		auto v = *value;
		auto v1 = pointers_change.getv(v);
		if(!v1)
			v1 = v;
		for(auto& e : pointers) {
			auto i = e.indexof(v);
			if(i != -1) {
				pid = (j << 24) | i;
				break;
			}
			j++;
		}
		source.write(&pid, sizeof(pid));
	} else {
		*value = 0;
		source.read(&pid, sizeof(pid));
		if(pid != -1) {
			auto bi = pid >> 24;
			auto ii = pid & 0xFFFFFF;
			auto v = pointers[bi].get(ii);
			auto v1 = pointers_change.get(v);
			if(!v1)
				v1 = v;
			*value = v1;
		}
	}
}

void archive::set(void* value, unsigned size) {
	if(writemode)
		source.write(value, size);
	else
		source.read(value, size);
}