#include <pservlet.h>
#include <pstd.h>
#include <string.h>
#include <stdlib.h>

static pipe_t name, line_no, out;
static char* what;

static int init(uint32_t argc, char const* const* argv, void* mem)
{
	if(argc != 2) return ERROR_CODE(int);

	name = pipe_define("name", PIPE_INPUT, NULL);
	line_no = pipe_define("line_no", PIPE_INPUT, NULL);
	out = pipe_define("out", PIPE_OUTPUT, NULL);

	what = strdup(argv[1]);

	return 0;
}

static int exec(void* mem)
{
	char name_buf[128] = {};
	size_t count = pipe_read(name, name_buf, sizeof(name_buf) - 1);
	name_buf[count] = 0;

	int num;
	pipe_read(line_no, &num, sizeof(int));

	pstd_bio_t* bio = pstd_bio_new(out);
	pstd_bio_printf(bio, "[%d] %s, %s\r\n", num, what, name_buf);
	pstd_bio_free(bio);

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
