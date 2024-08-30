#ifndef INIT
#define INIT

#include "base.h"
#include "copy.h"

unsigned long int get_src(const struct path& file);

void register_sync(const struct syncstat& syncstat, const unsigned long int& dest_index, const short& type);

void fill_base(void);

int init_program(void);

#endif // INIT
