# Be Modular
*Combining multiple servlets*

---

In the last chapter, we read the first line from the TCP connection as the name of the client and send greeting message to the client with its name. Now let's think about how can we make it better. 

In fact, parsing the first line seems reusable module and we may want to use it differently. So we can split the original servlet into two, one reads the TCP connection and extract the first line, another reads name and response the greeting message. 

## The getline servlet

Now we want to make the logic: Getting the first line from input as an independent servlet.So it should have two ports *in* which reads the input, *line* which is the first line of the input.

```C
static pipe_t in, out;
static int init(uint32_t argc, char const* const* argv, void* mem)
{
	in = pipe_define("in", PIPE_INPUT, NULL);
	out = pipe_define("line", PIPE_OUTPUT, NULL);
	return 0;
}
```

And the exec callback is just like the code in the last chapter.

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

	pipe_write(out, name, count);

	return 0;
}
```

## The say-hello servlet

Instead of just reading the first line, the *name* port only has the name. 

```C
static int exec(void* mem)
{
	char name[128] = {};

	size_t count = pipe_read(in, name, sizeof(name));

	pipe_write(out, what, strlen(what));
	pipe_write(out, ", ", 2);
	pipe_write(out, name, count);

	return 0;
}
```

## Change to the PSS Script

This time we have two servlet, thus we need to change our pipeline definition. 
We should have two different servlet, one is `getline` another is `say` and need to
bind the *line* port of `getline` servlet with the *name* port of `say`. 

```javascript
Service.start({
	read_name := "getline";
	say_hello := @"say Hello";
	() -> "in" read_name "line" -> "name" say_hello "out" -> ();
});

```


