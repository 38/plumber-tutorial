#include <pservlet.h>
#include <string.h>
#include <stdlib.h>
#include <pstd.h>

static pipe_t in, out, line_no;

static pstd_type_accessor_t line_no_acc;
static pstd_type_model_t*   type_model;

typedef struct {
	int line_num;
} state_t;

static int init(uint32_t argc, char const* const* argv, void* mem)
{
	in = pipe_define("in", PIPE_INPUT | PIPE_PERSIST, NULL);
	out = pipe_define("line", PIPE_OUTPUT, NULL);
	line_no = pipe_define("line_no", PIPE_OUTPUT, "int32");

	type_model = pstd_type_model_new();
	line_no_acc = pstd_type_model_get_accessor(type_model, line_no, "value");

	return 0;
}

static int free_state(void* state)
{
	free(state);
	return 0;
}

static int exec(void* mem)
{
	pstd_type_instance_t* inst = PSTD_TYPE_INSTANCE_LOCAL_NEW(type_model);

	char name[128] = {};

	state_t* state;

	/* First of all we try to pop the previously attached state with the communication resource */
	pipe_cntl(in, PIPE_CNTL_POP_STATE, &state);
	/* If it doesn't have previous state, we need to allocate a new one */
	if(NULL == state)
		state = calloc(sizeof(state_t), 1);

	size_t count = 0;
	int eof_rc;
	while(!(eof_rc = pipe_eof(in)))
	{
		size_t read = pipe_read(in, name + count, 1);
		if(name[count] == '\r' || name[count] == '\n') 
			break;
		count += read;
	}

	/* Increment the line number */
	if(count > 0)
	{
		state->line_num ++;

		pipe_write(out, name, count);

		PSTD_TYPE_INST_WRITE_PRIMITIVE(inst, line_no_acc, state->line_num);
	}

	/* Finally, attach the state with the communication resource again */
	if(eof_rc)
		pipe_cntl(in, PIPE_CNTL_CLR_FLAG, PIPE_PERSIST);
	else
		pipe_cntl(in, PIPE_CNTL_PUSH_STATE, state, free_state);

	pstd_type_instance_free(inst);

	return 0;
}

SERVLET_DEF = {
	.desc = "Read a single line from in",
	.init = init,
	.exec = exec,
};
