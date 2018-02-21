# Be Typed
*Typed pipe and primitive types*

---

In the previous example, we uses binary format to passing the line number. This makes the servlet easy to implement
and avoids the serialize and deserialize overhead. However it's a very dangerous, since a misconfigured socket server
can still start without error.

For example, in the `say-hello` servlet, we assume that the `line_no` port passing a integer but nobody enforce that.
Suppose we mistakely connected the line content port of the `getline` servlet to the line number port of `say-hello`
servlet. The socket server will still be able to start, but the `say-hello` servlet reasd first 4 bytes as an integer from the actual line context. 

```javascript
Service.start({
	read_name := "getline";
	say_hello := @"say Hello";
	() -> "in" read_name {
		"line" -> "line_no";
		"line_no" -> "name";
	} say_hello "out" -> ();
});
```


Similarly if the `getline` servlet uses different data representation for the line number
for example use an 8-bit integer instead of a 32-bit one. The socket server still starts but the behavior is unexpected.

Obviously we shouldn't make a misconfigured socket server starts, so we need **strong-typed pipes**.

## Strong-Typed Pipes

Actually all pipes in Plumber are strong-typed. You may have realized when using `pstest` to inspect the pipe ports
of a servlet, the program outputs type information as well.

```
$ pstest -s out -l getline 
Name    : getline
Desc    : Read a single line from in
Version : 0x00000000
Pipes   : [ID]	Name                            Flags
          [ 0]	__null__:[plumber/base/Raw]     0x00010000(W)
          [ 1]	__error__:[plumber/base/Raw]    0x00010000(W)
          [ 2]	in:[plumber/base/Raw]           0x00020000(PR)
          [ 3]	line:[plumber/base/Raw]         0x00010000(W)

```

The `plumber/base/Raw` is the type name. This is the special type in plumber indicates the pipe should
assing raw binary data. That is why we just feel our servlet port is untyped. 

To address the problem we mentioned earlier, we need make the servlet with more specific type. In our case
we should make it a integer. The following code declares the `line_no` port with 32-bit signed integer type.

```C
line_no = pipe_define("line_no", PIPE_OUTPUT, "int32");
```

To access the typed pipe with the code, we should use a **type model** and a **field accessor**.

```C
pstd_type_model_t* type_model;
pstd_type_accessor_t line_no_acc;

type_model = pstd_type_model_new();
line_no_acc = pstd_type_model_get_accessor(type_model, line_no, "value");
```

`type_model` collect all the type information of current servlet and `line_no_acc` carries the method to access
the `value` of `line_no` pipe port. After this we can use the type model and type accessor to access a typed value.

```C
pstd_type_instance_t* inst = PSTD_TYPE_INSTANCE_LOCAL_NEW(type_model);

PSTD_TYPE_INST_WRITE_PRIMITIVE(inst, line_no_acc, state->line_num);

pstd_type_instance_free(inst);
```

The first line of code allocates the `type_instance` object for this exec callback invocation. 
A type instance is used as a buffer for typed data. We use macro `PSTD_TYPE_INST_WRITE_PRIMITIVE` to
write the integer value to the type accessor. Finally we dispose the type instance.

On the other hand, we do the same thing for the `say-hello` servlet. The only difference is we use 
`PSTD_TYPE_INST_READ_PRIMITIVE` to grab the value of line number.

```C
int num = PSTD_TYPE_INST_READ_PRIMITIVE(int, inst, line_no_acc);
```

## Type Checking

Plumber framework performes type checking every time the servlet graph is about to start.
After the change, the PSS code we shown at the beginging of the code will not start and produce such error message.
It basically says the code is trying to convert a raw byte stream to a integer, which is not allowed in Plumber.

```
E[1519224222.644402|_solve_ces@src/sched/type.c:400] Invalid conversion: plumber/base/Raw -> int32
```


