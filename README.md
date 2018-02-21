# Hello World!

---

## Intro
Now let's start our tour of Plumber. Just like most of the tutorial, let's start with the hello world example.

A Plumber application is based on modular components and pipes, a modular component in Plumber is called **servlet**.
Similar to an executable in UNIX-like operating system, a **servlet** reads input from some **pipe ports** and
writes the output to some other ports. 
Unlike UNIX programs, pipe ports of a servlet needs requires a name for each port and the name is used for combining different
servlets into a entire system.

## Hello World
In this chapter, we are going to build a servlet saying hello to the world. Although this servlet doesn't really take any input,
normally a servlet should have at least one input and one output. Once we put the servlet into a real application, the servlet
code will be activated only when there's data waiting for read in the input port.

In our example, we defines an input port named *in* and an output port named *out*. We don't really use input port and write string
`"hello world"` to the *out* port.

First of all, we need two variable for the port identifer.

```C
static pipe_t in, out;
```

When each servlet is loaded by the Plumber framework, the init callback defined in the servlet will be invoked. In the init callback,
we can actually define the property of each pipe port using the framework API function `pipe_define`. 
In our example, we defined two pipe ports, *in* as input port and *out* as output port.
The last line we need to return 0, which indicates our servlet initialization is successfully done. 

```C
static int init(uint32_t argc, char const* const* argv, void* mem)
{
	in = pipe_define("in", PIPE_INPUT, NULL);
	out = pipe_define("out", PIPE_OUTPUT, NULL);
	return 0;
}
```

The next part of our servlet is the "main" function we called **exec** callback. Once there's any input pipe is active, the exec callback
will gets invoked by the Plumber runtime. In our case, the exec callback will be called whenever the *in* port is active.

```C
static int exec(void* mem)
{
	pipe_write(out, "Hello World", 11);
	return 0;
}
```

The last part of our servlet is the metadata section, it's used for the Plumber runtime to load and run the servlet.

```C
SERVLET_DEF = {
	.desc = "Say Hello World",
	.init = init,
	.exec = exec
};
```

Check `hello.c` for the actual implementation of the code.

## Servlet Build System

We have completed our first servlet, but we still need to build our servlet. Plumber framework provides a simple build system for
servlet build. To use the build system, we create a `build.mk` file as following.

```makefile
#build.mk
OUTPUT=out/
SRC=hello.c
TARGET=hello
LINKER=gcc
```

With `build.mk`, the build system will be able to build the servlet for us.

```
make -f ${ENVROOT}/lib/plumber/servlet.mk
```

## Try Our Servlet

Now we have created our very first servlet. How can we try it ? 

```
$ pstest -s out -p in=/dev/null,out=/dev/stdout hello
Hello World
```

The `pstest` is the tool provided by plumber framework to try a servlet quickly, `-s` param indicates where to find the servlet
`-p` param maps the pipe port to an actual file thus we can run the servlet as a normal program and the last param is the name of
the servlet.

Also we can inspect what ports the servlet has defined using the following command.

```
$ pstest -s out -l hello
Name    : hello
Desc    : Say Hello World
Version : 0x00000000
Pipes   : [ID]	Name                            Flags
          [ 0]	__null__:[plumber/base/Raw]     0x00010000(W)
          [ 1]	__error__:[plumber/base/Raw]    0x00010000(W)
          [ 2]	in:[plumber/base/Raw]           0x00000000(R)
          [ 3]	out:[plumber/base/Raw]          0x00010000(W)
```
