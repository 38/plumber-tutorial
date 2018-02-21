#include <pservlet.h>
#include <string.h>
#include <stdlib.h>

static pipe_t in, out;
static char* what;

static int init(uint32_t argc, char const* const* argv, void* mem)
{
	if(argc != 2) return ERROR_CODE(int);

	in = pipe_define("in", PIPE_INPUT, NULL);
	out = pipe_define("out", PIPE_OUTPUT, NULL);

	what = strdup(argv[1]);

	return 0;
}

static int exec(void* mem)
{
	char name[128] = {};
	size_t count = 0;
	while(!pipe_eof(in))
	{
		size_t read = pipe_read(in, name + count, 1);
		if(name[count] == '\r' || name[count] == '\n') 
			break;
		count += read;
	}

	pipe_write(out, what, strlen(what));
	pipe_write(out, ", ", 2);
	pipe_write(out, name, count);

	return 0;
}

static int unload(void* mem)
{
	free(what);
	return 0;
}

SERVLET_DEF = {
	.desc = "Say Some Words and Your Name",
	.init = init,
	.exec = exec,
	.unload = unload
};
