extern "C" void* malloc(unsigned size);
extern "C" void* realloc(void *ptr, unsigned size);
extern "C" void	free(void* pointer);

void* rmreserve(void* ptr, unsigned size) {
	if(!size) {
		if(ptr)
			free(ptr);
		return 0;
	}
	if(!ptr)
		return malloc(size);
	return realloc(ptr, size);
}

unsigned rmoptimal(unsigned need_count) {
	const unsigned mc = 256 * 256 * 256;
	unsigned m = 64;
	while(m < mc) {
		if(need_count < m)
			return m;
		m = m << 1;
	}
	return m;
}