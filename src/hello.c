#include <pservlet.h>

static pipe_t in, out;

static int init(uint32_t argc, char const* const* argv, void* mem)
{
	in = pipe_define("in", PIPE_INPUT, NULL);
	out = pipe_define("out", PIPE_OUTPUT, NULL);

	return 0;
}

static int exec(void* mem)
{
	pipe_write(out, "Hello World", 11);
	return 0;
}

SERVLET_DEF = {
	.desc = "Say Hello World",
	.init = init,
	.exec = exec
};
