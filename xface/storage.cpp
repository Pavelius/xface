#include "crt.h"
#include "stringcreator.h"
#include "storage.h"

int	storage::get() const {
	switch(size) {
	case sizeof(int) : return *((int*)data);
	case sizeof(short) : return *((short*)data);
	case sizeof(char) : return *((char*)data);
	default: return 0;
	}
}

void storage::set(const char* result) const {
	switch(type) {
	case Text:
		zcpy((char*)data, result, size);
		break;
	case TextPtr:
		*((const char**)data) = szdup(result);
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