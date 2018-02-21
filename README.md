# Say a Different Word
*the Servlet Initialization String*

---

## Generialize our first servlet

Servlets are basic build blocks in the world of Plumber. Our very first servlet only says `hello world`,
but what if we want to say anything other than hello? Do we need to create different servlets saying different
words? The answer is obviously no. Look at how we define our servlet's init callback.

```C
static int init(uint32_t argc, char const* const* argv, void* mem);
```

Actually, we are able to pass a **servlet init parameter** to the servlet, thus the servlet can be initialized 
differently. Very similar to a UNIX program, right?
 
In order to say whatever we want, we can pass the words we want to say via servlet init parameter. And instead of 
say hello, the servlet say whatever the init parameter is.

```C
static char* what;
static int init(uint32_t argc, char const* const* argv, void* mem)
{
	if(argc != 2) return ERROR_CODE(int);
	in = pipe_define("in", PIPE_INPUT, NULL);
	out = pipe_define("out", PIPE_OUTPUT, NULL);
	what = strdup(argv[1]);
	return 0;
}
```

We also check if we have exactly one additional servlet init parameter, otherwise, we return `ERROR_CODE(int)`
which indicates the servlet is not able to initialize.

In the exec callback, instead of saying hello, say the string stored in `what`.

```C
static int exec(void* mem)
{
	pipe_write(out, what, strlen(what));
	return 0;
}
```

One additional thing we need to do is disposing the `what` variable when the server is shutting down. So we define
a unload servlet callback as following.

```C
static int unload(void* mem)
{
	free(what);
	return 0;
}
```

Finally, we put unload callback in the metadata.

```C
SERVLET_DEF = {
	.desc = "Say Some Words",
	.init = init,
	.exec = exec,
	.unload = unload
};
```

## Change the PSS script

Since how the servlet is renamed to say and accept exactly one initialization parameter, our socket server won't work.
To make the entire server work again, we need to change the PSS script a little bit as well. 
First, we need change the name of the servlet from `hello` to `say`. Second, we need to pass additional parameter to the
servlet. Here's how we do this.

```javascript
Service.start({
	say_hello := @"say Hello\ World";
	() -> "in" say_hello "out" -> ();
});
```

Just as UNIX shells, we need the escape sequence that prevent the word `Hello World` being tokenized into two parts.
