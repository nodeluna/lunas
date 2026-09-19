#pragma once

#include <unistd.h>
#include <sys/ioctl.h>

namespace lunas
{
	struct termsize {
			unsigned short ts_row;
			unsigned short ts_col;
			unsigned short ts_xpixel;
			unsigned short ts_ypixel;
	};

	inline struct termsize terminal_size(void)
	{
		struct winsize term_size;
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &term_size);

		struct termsize termsize;
		termsize.ts_row	   = term_size.ws_row;
		termsize.ts_col	   = term_size.ws_col;
		termsize.ts_xpixel = term_size.ws_xpixel;
		termsize.ts_ypixel = term_size.ws_ypixel;
		return termsize;
	}
}
