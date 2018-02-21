#include <pservlet.h>
#include <string.h>
#include <stdlib.h>

static pipe_t in, out;

static int init(uint32_t argc, char const* const* argv, void* mem)
{
	in = pipe_define("in", PIPE_INPUT | PIPE_PERSIST, NULL);
	out = pipe_define("line", PIPE_OUTPUT, NULL);
	return 0;
}

static int exec(void* mem)
{
	char name[128] = {};
	
	size_t count = 0;
	int eof_rc;
	while(!(eof_rc = pipe_eof(in)))
	{
		size_t read = pipe_read(in, name + count, 1);
		if(name[count] == '\r' || name[count] == '\n') 
			break;
		count += read;
	}

	pipe_write(out, name, count);

	return 0;
}

SERVLET_DEF = {
	.desc = "Read a single line from in",
	.init = init,
	.exec = exec,
};
