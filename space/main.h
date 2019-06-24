#pragma once
template<typename T> struct bsmeta {
	static T			elements[];
};
struct combati {
	char				miss;
	char				light;
	char				heavy;
	char				fatal;
};
struct weaponi {
	const char*			name;
	combati				combat;
};
struct armori {
	const char*			name;
	combati				combat;
};