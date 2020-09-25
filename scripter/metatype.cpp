#include "main.h"

using namespace code;

BSDATA(metatypei) = {{"Static", "Статический"},
{"Public", "Публичный"},
{"Dimension", "Измерение"},
{"Scalar", "Скалярный тип"},
};
assert_enum(metatype, ScalarType)