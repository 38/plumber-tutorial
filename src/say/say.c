#include <pservlet.h>
#include <pstd.h>
#include <string.h>
#include <stdlib.h>
#include <pstd/types/string.h>

static pipe_t name,  out;
static char* what;

static pstd_type_accessor_t line_no_acc;
static pstd_type_accessor_t line_acc;
static pstd_type_model_t*   type_model;

static int init(uint32_t argc, char const* const* argv, void* mem)
{
	if(argc != 2) return ERROR_CODE(int);

	name = pipe_define("name", PIPE_INPUT, "plumber_tutorial/Line");
	out = pipe_define("out", PIPE_OUTPUT, NULL);

	what = strdup(argv[1]);
	
	type_model = pstd_type_model_new();
	line_no_acc = pstd_type_model_get_accessor(type_model, name, "number");
	line_acc = pstd_type_model_get_accessor(type_model, name, "content.token");

	return 0;
}

static int exec(void* mem)
{
	pstd_type_instance_t* inst = PSTD_TYPE_INSTANCE_LOCAL_NEW(type_model);

	int num = PSTD_TYPE_INST_READ_PRIMITIVE(int, inst, line_no_acc);
	scope_token_t token = PSTD_TYPE_INST_READ_PRIMITIVE(scope_token_t, inst, line_acc);

	const pstd_string_t* str_obj = pstd_string_from_rls(token);
	const char* name_buf = pstd_string_value(str_obj);

	pstd_bio_t* bio = pstd_bio_new(out);
	pstd_bio_printf(bio, "[%d] %s, %s\r\n", num, what, name_buf);
	pstd_bio_free(bio);
	
	pstd_type_instance_free(inst);

	return 0;
}

static int unload(void* mem)
{
	free(what);
	pstd_type_model_free(type_model);
	return 0;
}

SERVLET_DEF = {
	.desc = "Say Some Words and Your Name",
	.init = init,
	.exec = exec,
	.unload = unload
};
