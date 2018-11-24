#include "datetime.h"

datetime::datetime(int year, int month, int day, int hour, int minute) : d(((1461 * (year - 1600 + (month - 14) / 12)) / 4
	+ (367 * (month - 2 - 12 * ((month - 14) / 12))) / 12
	- (3 * ((year - 1600 + 100 + (month - 14) / 12) / 100)) / 4
	+ day - 32075) * 1440
	+ hour * 60 + minute) {
}

bool datetime::isleap() const {
	const auto year = this->year();
	return (year % 400) == 0 || (((year % 100) != 0) && (year % 4) == 0);
}

int datetime::year() const {
	unsigned ell, n, i, j;
	ell = (d / 1440) + 68569;
	n = (4 * ell) / 146097;
	ell = ell - (146097 * n + 3) / 4;
	i = (4000 * (ell + 1)) / 1461001;
	ell = ell - (1461 * i) / 4 + 31;
	j = (80 * ell) / 2447;
	ell = j / 11;
	return (int)(100 * (n - 49) + i + ell + 6400);
}

int datetime::month() const {
	unsigned ell, n, i, j;
	ell = (d / 1440) + 68569;
	n = (4 * ell) / 146097;
	ell = ell - (146097 * n + 3) / 4;
	i = (4000 * (ell + 1)) / 1461001;
	ell = ell - (1461 * i) / 4 + 31;
	j = (80 * ell) / 2447;
	ell = j / 11;
	return (int)(j + 2 - (12 * ell));
}

int	datetime::day() const {
	unsigned ell, n, i, j;
	ell = (d / 1440) + 68569;
	n = (4 * ell) / 146097;
	ell = ell - (146097 * n + 3) / 4;
	i = (4000 * (ell + 1)) / 1461001;
	ell = ell - (1461 * i) / 4 + 31;
	j = (80 * ell) / 2447;
	return (int)(ell - (2447 * j) / 80);
}