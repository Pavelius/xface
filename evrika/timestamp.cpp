#include "main.h"

static unsigned			global_counter;
static short unsigned	global_cluster;

void timestamp::generate() {
	create_date = datetime::now();
	counter = global_counter++;
}