# Be Stateful

---

Last time we make our socket interactive, the server and client can take multiple turns of communication. But most of the interactive socket servers are stateful. For example, an old telnet base BBS application, the user needs to login first and then the connection authenticated to the logged in user, so the user can use the connection for posting messages to the BBS. 

In this chapter, we also want to modify our server into a stateful fashion. What we are going to do is add a line number before the greeting message.

```
$ telnet localhost 12345
Trying 127.0.0.1...
Connected to localhost.
Escape character is '^]'.
a
[1] Hello, a
b
[2] Hello, b
c
[3] Hello, c
d
[4] Hello, d
e
[5] Hello, e
^]

telnet> q
Connection closed.
```

# Communication resource and state

To implement the state, we need the concept of communication resource once again. 
The last chapter explained how we can make the Plumber infrastructure keep the 
connection for us, this time we introduce another useful Plumber framework API
which can attach a state variable to the communication resource, and in this way,
the state is perserved accross the different communication turns in the same
TCP connection.

To archieve this we need the help of API `pipe_cntl`.

```
int pipe_cntl(pipe_t pipe, uint32_t opcode, ...);
```

This function provides many functionality for pipe manipulation (even some IO module specific operations). But for this time, we just introduce two operations:

* `PIPE_CNTL_POP_STATE`: Pop the existing state attached to the communication resource
* `PIPE_CNTL_PUSH_STATE`: Attaching the new state to the communication resource

In our getline servlet, we need another output port passing the line number.

```C
line_no = pipe_define("line_no", PIPE_OUTPUT, NULL);
```
   
For each time the exec callback runs, before we move forward with the current line, we need to pop the existing state first. If there's no state attached, we need to initialize a new one.

```C
typedef struct {
	int line_num;
} state_t;

state_t* state;
pipe_cntl(in, PIPE_CNTL_POP_STATE, &state);
if(NULL == state) state = calloc(sizeof(state_t), 1);
```

After we parsed an entire line, we need to incrment the line number counter and push the state  back to the resource.

```C
state->line_num ++;

pipe_write(line_no, &state->line_num, sizeof(int));

pipe_cntl(in, PIPE_CNTL_PUSH_STATE, state, free_state);
```

## libpstd - The Plumber Standard Utilies

In this example, we also uses the libpstd, the utily library for building a servlet. It provides some high level features and wrapper to the raw Plumber framework API. For this time, we used the bufferred IO functions `pstd_bio_*` in our say servlet.

Like stdio library in standard C, the `pstd_bio_*` function provides the functionality of Bufferred IO which is helpful to improve the performance. At the same time, it also provides higher level functions like `pstd_bio_printf`. 

Back to the say-hello servlet, we should make the say-hello servlet takes an additional input - the line number.

```C
line_no = pipe_define("line_no", PIPE_INPUT, NULL);
```

For each time the exec callback runs, we should read the line number as well.

```C
int num;
pipe_read(line_no, &num, sizeof(int));
```

Finally, we render the output with `pstd_bio_printf`

```C
	pstd_bio_t* bio = pstd_bio_new(out);
	pstd_bio_printf(bio, "[%d] %s, %s\r\n", num, what, name_buf);
	pstd_bio_free(bio);
```

## Test Our Server

The state is attached with each connection, thus we should be able to have multiple client connect to the server at the same time. Each of the client should see it's own state.

![screenshot](https://raw.githubusercontent.com/38/plumber-tutorial/7-stateful/path/test.gif)
