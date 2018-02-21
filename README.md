# Interactive

---

## A Interactive Socket Server

In the previous example, our socket server closes the connection after we input the name. 
For the TCP IO module, it treat one connection as one pair of input port and output port, so after the connection is handled by our application, the TCP connection will be closed by the TCP IO module. But sometime, we may want to implement a interactive socket server, which means after the client sends the first line of data, the server response the server and wait for client sending the next line of data.

For example, in our say hello socket server, we may want the read multiple names from the same telnet connection and send greeting information for each lines of the name.

The following telnet output illustrate this interactive procedure. After we input `plumber`, the server response with `Hello, Plumber` and wait the client sending another name.

```
$ telnet localhost 12345
Trying 127.0.0.1...
Connected to localhost.
Escape character is '^]'.
Plumber
Hello, Plumber
world
Hello, world
世界
Hello, 世界
^]
telnet> q
Connection closed.
```

## Pipe Flags

In the previous chapter, we have used the function `pipe_define` to define a named pipe port. We can define the input port with `PIPE_INPUT` flag and `PIPE_OUTPUT` flag for the output port.

```
pipe_define("input", PIPE_INPUT, NULL);
pipe_define("output", PIPE_OUTPUT, NULL);
```

In fact there are few more pipe flags that modifies the behavior of pipe.

| flag | Effect         |
| ---- | -------------- |
| PIPE\_INPUT | Declare the port as an input port |
| PIPE\_OUTPUT | Declare the port as an output port |
| PIPE\_ASYNC  | Enable the asynchronus write function |
| PIPE\_SHADOW | Declare the output is a duplication of another port |
| PIPE\_PERSIST | Suggest the Plumber framework hold the underlying resource for reuse|

This time, we are not going to discuss them all. Instead we forcus on the `PIPE\_PERSISTENT` flag, which suggests the Plumber framework keep the underlying resource. 

Although a servlet doesn't actually manipulate any communication resources such as TCP connections, but the pipe port is an abstraction of some kinds of communication resources. Each time the exec callback of the servlet is invoked, each of the pipe port has an underlying communication resources. For example, for a exec callback invocation triggered by a TCP connection, the underlying resource should be the TCP socket. For the exec callback invocation triggered by upstream servlet's output, the underlying resources should be memory, etc. The persistent pipe flag give the Plumber infrastructure the hint" the resource might be useful in the future.

Translate the interactive socket server into Plumber term, we could say the input pipe has persistent flag.

In order to do so, we only needs to change one line of code.

```C
in = pipe_define("in", PIPE_INPUT | PIPE_PERSISTENT, NULL);
```
