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

const char* storage::get(char* result, const char* result_end, bool force_to_result) const {
	switch(type) {
	case Text:
		if(!force_to_result)
			return (const char*)data;
		else {
			stringcreator sc(result, result_end);
			sc.add((const char*)data);
		}
		break;
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