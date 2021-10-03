#ifndef __robin_c_io
#define __robin_c_io


#include <fcntl.h>  // open(2)
#include <unistd.h> // read(2)

#include "c.h"
#include "string_builder.h"


external b32 io_read_file(c8 **dst, const c8 *filename);


b32 io_read_file(c8 **dst, const c8 *filename) {
	s32 fd, n;
	string_builder *builder;
	c8 buffer[1024];
	
	fd = open(filename, O_RDONLY);
	if (fd == 0) {
		return false;
	}

	builder = string_make_builder();

	for (;;) {
		n = read(fd, buffer, 1024);
		if (n == 0) {
			break;
		}
		string_write_n(builder, buffer, n);
	}

	*dst = string_builder_to_c(builder);
	string_free_builder(builder);
	return true;
}


#endif // __robin_c_io
