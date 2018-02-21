#include <pservlet.h>
#include <pstd.h>
#include <string.h>
#include <stdlib.h>
#include <pstd/types/string.h>

typedef struct {
	pipe_t name,  out;
	char* what;
	pstd_type_accessor_t line_no_acc;
	pstd_type_accessor_t line_acc;
	pstd_type_model_t*   type_model;
} servlet_context_t;

static int init(uint32_t argc, char const* const* argv, void* mem)
{
	servlet_context_t* ctx = (servlet_context_t*)mem;
	if(argc != 2) return ERROR_CODE(int);

	ctx->name = pipe_define("name", PIPE_INPUT, "plumber_tutorial/Line");
	ctx->out = pipe_define("out", PIPE_OUTPUT, NULL);

	ctx->what = strdup(argv[1]);
	
	ctx->type_model = pstd_type_model_new();
	ctx->line_no_acc = pstd_type_model_get_accessor(ctx->type_model, ctx->name, "number");
	ctx->line_acc = pstd_type_model_get_accessor(ctx->type_model, ctx->name, "content.token");

	return 0;
}

static int exec(void* mem)
{
	servlet_context_t* ctx = (servlet_context_t*)mem;
	pstd_type_instance_t* inst = PSTD_TYPE_INSTANCE_LOCAL_NEW(ctx->type_model);

	int num = PSTD_TYPE_INST_READ_PRIMITIVE(int, inst, ctx->line_no_acc);
	scope_token_t token = PSTD_TYPE_INST_READ_PRIMITIVE(scope_token_t, inst, ctx->line_acc);

	const pstd_string_t* str_obj = pstd_string_from_rls(token);
	const char* name_buf = pstd_string_value(str_obj);

	pstd_bio_t* bio = pstd_bio_new(ctx->out);
	pstd_bio_printf(bio, "[%d] %s, %s\r\n", num, ctx->what, name_buf);
	pstd_bio_free(bio);
	
	pstd_type_instance_free(inst);

	return 0;
}

static int unload(void* mem)
{
	servlet_context_t* ctx = (servlet_context_t*)mem;
	free(ctx->what);
	pstd_type_model_free(ctx->type_model);
	return 0;
}

SERVLET_DEF = {
	.desc = "Say Some Words and Your Name",
	.size = sizeof(servlet_context_t),
	.init = init,
	.exec = exec,
	.unload = unload
};
