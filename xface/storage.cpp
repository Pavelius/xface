#include "crt.h"
#include "stringcreator.h"
#include "storage.h"

int	storage::get() const {
	switch(size) {
	case 4: return *((int*)data);
	case 2: return *((short*)data);
	case 1: return *((char*)data);
	default: return 0;
	}
}

void storage::set(int value) const {
	switch(size) {
	case 4: *((int*)data) = value; break;
	case 2: *((short*)data) = value; break;
	case 1: *((char*)data) = value; break;
	}
}

void storage::set(const char* result) const {
	int number;
	switch(type) {
	case Text:
		zcpy((char*)data, result, size);
		break;
	case TextPtr:
		*((const char**)data) = szdup(result);
		break;
	case Number:
		number = sz2num(result);
		set(number);
		break;
	}
}

void storage::getf(char* result, const char* result_end) const {
	auto p = get(result, result_end);
	if(p != result) {
		stringcreator sc(result, result_end);
		sc.add(p);
	}
}

const char* storage::get(char* result, const char* result_end) const {
	const char* text;
	switch(type) {
	case Text:
		return (const char*)data;
	case TextPtr:
		text = *((const char**)data);
		if(!text)
			return "";
		return text;
	case Number:
	case Bool:
		if(true) {
			stringcreator sc(result, result_end);
			sc.add("%1i", get());
		}
		break;
	default:
		result[0] = 0;
		break;
	}
	return result;
}