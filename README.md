# Servlet Context
*Multi-instance and Standard Servlets*

*See [diff](https://github.com/38/plumber-tutorial/compare/9-structured-data-type-and-RLS-src...10-reusable-servlet-and-standard-servlets) since last chapter*

---

This time, let's add more things to our socket server. We are going to make the servlet say differently to different name. For example, if the client's name is `"Plumber"`, we should say `"Hello, Plumber"`. Otherwise the socket server should say `"Greeting from Plumber"`. 

In order to do this, we need two `say-hello` servlet which return different greeting strings. But think about our servlet implementation, the string to say is a global variable, which means we can't have multiple string to say. To resolve this issue, let's talk about the servlet context first.

## Servlet Context

Plumber allows the same servlet being added to multiple times in the same pipe binding graph. For example, we can have `say Hello` and `say Greeting From Plumber` in the same binding graph. In this senario, the global variable will be broken, since all the instance of the same servlet shares the same global variable. 

In order to make each instance of servlet can hold it's local data, Plumber framework provides each servlet instance a unique memory region called servlet context. Usually all the servlet instance data should be in the servlet context. In our example, the servlet context should have the following structure.

```C
typedef struct {
	pipe_t name,  out;
	char* what;
	pstd_type_accessor_t line_no_acc;
	pstd_type_accessor_t line_acc;
	pstd_type_model_t*   type_model;
} servlet_context_t;
```

In order to tell Plumber framework how many bytes to allocate for the servlet context.

```C
SERVLET_DEF = {
	.desc = "Say Some Words and Your Name",
	.size = sizeof(servlet_context_t),
	.init = init,
	.exec = exec,
	.unload = unload
};
```

Finally, we need to hold the pipe port definition, type model and string to say in the servlet context

```C
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
```

By doing so, we can have multiple instance of the same servlet in the same application.

## Standard Servlets

Plumber provides many standard servlet, include the dataflow branch. The following example shows the dataflow branch is configured to match if the string from the cond ports is `"Plumber"`. If it is, copy the content of `data` port to `out0` port, otherwise copy it to `default` port.

```
$ pstest -l dataflow/demux Plumber
Name    : dataflow/demux
Desc    : The demultiplexer, which takes N inputs and one condition, produces the copy of selected input
Version : 0x00000000
Pipes   : [ID]	Name                                       Flags
          [ 0]	__null__:[plumber/base/Raw]                0x00010000(W)
          [ 1]	__error__:[plumber/base/Raw]               0x00010000(W)
          [ 2]	cond:[plumber/std/request_local/String]    0x00000000(R)
          [ 3]	data:[$Tdata]                              0x00000000(R)
          [ 4]	out0:[$Tdata]                              0x00190003(SWD->3)
          [ 5]	default:[$Tdata]                           0x00190003(SWD->3)
```

To archieve our goal, we can use this servlet to decide which `say-hello` servlet should be used. 

```javascript
socket_server = {
  get_line      := "getline";
  greet_plumber := "say Hello";
  greet_others  : = "say Greeting\\ From\\ Plumber";
  selector      := "dataflow/demux Plumber";
  
  () -> "in" get_line "line" -> "in" selector {
    "out0" -> "line" greet_plumber;
    "default" -> "line" greet_others;
  };
}
```

We can see two problem, now we have two output ports, one from `greet_plumber`, one from `greet_others`, but we only have one output. So we need to merge two result into one, we have a standard servlet called `dataflow/firstnonempty` serves for this purpose. The follow example shows the servlet initialized with 2 input ports. If the input port `in0` has data, it copies the data to the `out` port. If the input port `in0` is empty and `in1` isn't, it copies the data from `in1` to the `out` port. Otherwise, the `out` port will be empty as well.

```
$ pstest -l dataflow/firstnonempty 2      
Name    : dataflow/firstnonempty
Desc    : Pick up the first non-empty input from N inputs
Version : 0x00000000
Pipes   : [ID]	Name                            Flags
          [ 0]	__null__:[plumber/base/Raw]     0x00010000(W)
          [ 1]	__error__:[plumber/base/Raw]    0x00010000(W)
          [ 2]	in0:[$T]                        0x00000000(R)
          [ 3]	in1:[$T]                        0x00000000(R)
          [ 4]	out:[$T]                        0x00010000(W)
```

With the `firstnonempty` servlet merge the result, we have only one part missing. How we can extract the line content from the `getline` servlet's output. The answer is use the `dataflow/extract` servlet, which is desgined to extract a field from a typed pipe. 

```
$ pstest -l dataflow/extract line
Name    : dataflow/extract
Desc    : Extract a field from the input
Version : 0x00000000
Pipes   : [ID]	Name                            Flags
          [ 0]	__null__:[plumber/base/Raw]     0x00010000(W)
          [ 1]	__error__:[plumber/base/Raw]    0x00010000(W)
          [ 2]	input:[$T]                      0x00000000(R)
          [ 3]	output:[$T.line]                0x00010000(W)
```

Finally we can put everything together.

```javascript
Service.start({
	/* Declare a node */
	read_name := "getline";
	selector  := "dataflow/demux Plumber";
	extract_name := "dataflow/extract content";
	say_hello_to_plumber := "say Hello";
	say_hello_to_others  := @"say Greeting\ from\ Plumber";
	merger := "dataflow/firstnonempty 2";
	/* Define the pipes */
	() -> "in" read_name {
		"line" -> "input" extract_name "output" -> "cond";
		"line" -> "data";
	} selector {
		"out0" -> "name" say_hello_to_plumber "out" -> "in0";
		"default" -> "name" say_hello_to_others "out" -> "in1";
	} merger "out" -> ();
});
```
