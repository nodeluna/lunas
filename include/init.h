#ifndef INIT
#define INIT

#include "base.h"
#include "copy.h"


#define OK_DEST 0
#define SAME_INPUT_PATH 1
#define NOT_DEST 2
#define EXISTING_DIR 3
#define TYPE_CONFLICT 4
#define SAME_MTIME 5
#define NO_SPACE_LEFT 6

#define OK_SRC 0
#define NO_SRC 1
#define BROKEN_SRC 2

unsigned long int get_src(const struct base::path& file);

void register_sync(const struct syncstat& syncstat, const unsigned long int& dest_index, const short& type);

void fill_base(void);

int avoid_src(const struct base::path& file, const unsigned long int& src_mtime_i);

int avoid_dest(const struct base::path& file, const struct metadata& metadata, const size_t& src_mtime_i, const size_t& dest_index);

int init_program(void);

#endif // INIT
