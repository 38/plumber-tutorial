#include <pservlet.h>
#include <string.h>
#include <stdlib.h>
#include <pstd.h>
#include <pstd/types/string.h>

typedef struct {
	pipe_t in, line;
	pstd_type_accessor_t line_no_acc;
	pstd_type_accessor_t line_acc;
	pstd_type_model_t*   type_model;
} servlet_context_t;

typedef struct {
	int line_num;
} state_t;

static int init(uint32_t argc, char const* const* argv, void* mem)
{
	servlet_context_t* ctx = (servlet_context_t*)mem;

	ctx->in = pipe_define("in", PIPE_INPUT | PIPE_PERSIST, NULL);
	ctx->line = pipe_define("line", PIPE_OUTPUT, "plumber_tutorial/Line");

	ctx->type_model = pstd_type_model_new();
	ctx->line_no_acc = pstd_type_model_get_accessor(ctx->type_model, ctx->line, "number");
	ctx->line_acc = pstd_type_model_get_accessor(ctx->type_model, ctx->line, "content.token");

	return 0;
}

static int free_state(void* state)
{
	free(state);
	return 0;
}

static int exec(void* mem)
{
	servlet_context_t* ctx = (servlet_context_t*)mem;
	
	pstd_type_instance_t* inst = PSTD_TYPE_INSTANCE_LOCAL_NEW(ctx->type_model);

	state_t* state;

	/* First of all we try to pop the previously attached state with the communication resource */
	pipe_cntl(ctx->in, PIPE_CNTL_POP_STATE, &state);
	/* If it doesn't have previous state, we need to allocate a new one */
	if(NULL == state)
		state = calloc(sizeof(state_t), 1);

	pstd_string_t* str_obj = pstd_string_new(128);

	size_t count = 0;
	int eof_rc;
	while(!(eof_rc = pipe_eof(ctx->in)))
	{
		char buf;
		size_t read = pipe_read(ctx->in, &buf, 1);
		if(read > 0)
		{
			if(buf == '\r' || buf == '\n') 
				break;
			pstd_string_write(str_obj, &buf, 1);
		}

		count += read;
	}

	/* Increment the line number */
	if(count > 0)
	{
		state->line_num ++;

		scope_token_t token = pstd_string_commit(str_obj);

		PSTD_TYPE_INST_WRITE_PRIMITIVE(inst, ctx->line_acc, token);
		PSTD_TYPE_INST_WRITE_PRIMITIVE(inst, ctx->line_no_acc, state->line_num);
	}
	else pstd_string_free(str_obj);

	/* Finally, attach the state with the communication resource again */
	if(eof_rc)
		pipe_cntl(ctx->in, PIPE_CNTL_CLR_FLAG, PIPE_PERSIST);
	else
		pipe_cntl(ctx->in, PIPE_CNTL_PUSH_STATE, state, free_state);

	pstd_type_instance_free(inst);

	return 0;
}

static int unload(void* mem)
{
	servlet_context_t* ctx = (servlet_context_t*)mem;
	pstd_type_model_free(ctx->type_model);
	return 0;
}

SERVLET_DEF = {
	.desc = "Read a single line from in",
	.size = sizeof(servlet_context_t),
	.init = init,
	.exec = exec,
	.unload = unload
};
