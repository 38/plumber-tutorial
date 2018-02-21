# Hello World Server
*the PSS Scripting Language*

---

We already have our first servlet, in this chapter let's make our first servlet a simple socket server.
We want the servlet serves network traffic. Whenever a TCP connection is established to the port it listening,
it says hello and close the connection. 

In order to make the servlet runs as a server, we need bind its input and output ports to a TCP port.
Plumber defines a simple configuration script calls PSS. And we are able to bind servlet ports easily with the script.

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

Line 1 and 2 loads the IO module with `insmod`, which is used as the input and output of the entire application. 

Line 4 imports the library to start a service and the last few lines defines a pipe binding graph and run the application.

## Run the application

Use the PSS interpreter to run the application.

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
