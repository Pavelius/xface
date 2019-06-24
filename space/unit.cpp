#include "main.h"

int uniti::getalive() const {
	return miss - fatal;
}

int uniti::getready() const {
	return getalive() - heavy;
}