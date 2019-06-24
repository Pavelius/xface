#include "main.h"

int uniti::getalive() const {
	return squad.miss - squad.fatal;
}

int uniti::getready() const {
	return getalive() - squad.heavy;
}