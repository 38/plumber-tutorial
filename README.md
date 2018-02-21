# Say Your Name
*Read from inputs*

*See [diff](https://github.com/38/plumber-tutorial/compare/3-say-different-words-src...4-be-modular-src) since last chapter*

---

Now we change our goal a little bit, when the socket server is connected by the client, it waits for the client say its name.
And the server will send the message with the client's name. For example:

```
$ telnet localhost 12345
Trying 127.0.0.1...
Connected to localhost.
Escape character is '^]'.
Plumber
Hello, PlumberConnection closed by foreign host.
```

So the servlet needs to read the *in* pipe before write the data from the first line. (In our example we don't really handles all the case just for simplicity)

```C
static int exec(void* mem)
{
	char name[128] = {};
	size_t count = 0;
	while(!pipe_eof(in))
	{
		size_t read = pipe_read(in, name + count, 1);
		if(name[count] == '\r' || name[count] == '\n')
			break;
		count += read;
	}

	pipe_write(out, what, strlen(what));
	pipe_write(out, ", ", 2);
	pipe_write(out, name, count);

	return 0;
}
```

The code above try to read the pipe data util it either exhuasted or a new line is recieved. And then we write the message with the name we read from *in*.
