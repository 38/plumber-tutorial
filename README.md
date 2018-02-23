# Hello World Server

[Prev](https://github.com/38/plumber-tutorial/blob/1-hello-world/README.md) |
[Tables of Content](https://github.com/38/plumber-tutorial/blob/master/README.md) | 
[Next](https://github.com/38/plumber-tutorial/blob/3-say-different-words/README.md)

*PSS Scripting Language*

*See [diff](https://github.com/38/plumber-tutorial/compare/1-hello-world-src...2-hello-world-server-src) since last chapter*

---

We already have our first servlet, in this chapter let's make our first servlet a simple socket server.
We want the servlet serves network traffic. Whenever a TCP connection is established to the port it listening,
it says hello and close the connection. 

In order to make the servlet runs as a server, we need bind its input and output ports to a TCP port.
Plumber defines a simple configuration script calls PScript. And we are able to bind servlet ports easily with the script.

```javascript
//server.pss
insmod("mem_pipe");
insmod("tcp_pipe 12345");

import("service");

runtime.servlet.path += ":out/"

Service.start({
	say_hello := "hello";
	() -> "in" say_hello "out" -> ();
});
```

Line 1 and 2 loads the IO module with `insmod`. `tcp_pipe` module actually handles the network traffic and `mem_pipe` module is the default communication method between different servlets in the same application.

Line 4 imports a PSS library to start a service and the last few lines defines a pipe binding graph and run the application.

## PScript Language

Let's talk more about the PScript language. The name is short for Plumber Script or Pipe Script.
The PSS is the configurator and shell of Plumber infrastructure.
PScript is responsible to setup all the pipelines, configure all the parameters and launch the application.
A PScript program is the configure file in Plumber.

Unlike most of the confgure file, PScript is a Turing-completed programming language which allows developer create
application generator with it. In later this tutorial we will demostrate how to use PScript generate application dynamically.

The PScript intepreter has REPL mode, which allows people to play with the language interactively.

```
$ pscript

REPL Shell for Plumber Service Script

Plumber Version [0.1.1.f70c9a1 2018-02-21T20:34:20 Linux]

help()  -> Get the help message
quit()  -> Quit the interactive client

PSS> 

```

The syntax of the PScript language is mostly borrowed and simplified from JavaScript, but there are few difference:

- Object oriented is not supported. PScript don't have prototype and `new` operator, etc.
- Only way to access dictionary is use subscript (e.g. `map["key"] = "value"`). Expression such as `map.key = "value"` is not supported. 
- All functions are annonymous, the identifer after the keyword `function` is used as debugging info rather than the identifier.
- More strict limit to `undefined` value, only `==` and `!=` operator is allowed for `undefined`

## Run the application

Use the PScript interpreter to run the application.

```
pscript server.pss
```

We can use `telnet localhost 12345` to try our first server.

```
$ telnet localhost 12345
Trying 127.0.0.1...
Connected to localhost.
Escape character is '^]'.

Hello WorldConnection closed by foreign host.
```
