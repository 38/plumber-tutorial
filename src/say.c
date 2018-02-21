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
	pipe_write(out, what, strlen(what));
	return 0;
}

static int unload(void* mem)
{
	free(what);
	return 0;
}

SERVLET_DEF = {
	.desc = "Say Some Words",
	.init = init,
	.exec = exec,
	.unload = unload
};
