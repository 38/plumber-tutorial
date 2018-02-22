# Structured Data

[Prev](https://github.com/38/plumber-tutorial/blob/8-be-typed-src/README.md) | 
[Tables of Content](https://github.com/38/plumber-tutorial/blob/master/README.md) | 
[Next](https://github.com/38/plumber-tutorial/blob/10-servlet-context/README.md)

*protocol managemnet and local object*

*See [diff](https://github.com/38/plumber-tutorial/compare/8-be-typed-src...9-structured-data-type-and-RLS-src) since last chapter*

---

In the last example, we make the line number pipe strong-typed and transmits a integer via the pipe. 
This works fine for our example, however, when the servlet needs to handle many primitives, it's hard to
implement all the primitives as different typed pipes. Thus sometimes we want to send a pack of structured
data which contains several data fields in the data pack. For example, suppose we have a servlet which produces
the RGB color value as output, it's reasonable that we send the three color values via the same pipe port.

## Centralized Protocol System

The easiest way to send a pack of values is write the C struct directly into a untyped pipe, but we lose the benifit we
gained from type. Even we send a JSON string, it's hard to detect if two bound pipe ports are actually talking in the same
way. What makes things even worse, once we have multiple versions of the same servlet, it's really hard to dealing with the
compatibility issue. 

To address those protocol issue, Plumber introduces a centrialized protocol management system. Instead of letting the servlet
itself define the data structure, Plumber's protocol system manages all the protocols using between different servlets.
Thus each servlet only declare the pipe with the type name and the field of the type is accessed using the **type accessor**.
The  following code demonstrate how to get a type accessor for the field named `red` in the `color` pipe port.

```C

color = pipe_define("color", PIPE_OUTPUT, "example_type/Color");

pstd_type_model_t* type_model = pstd_type_model_new();

red_accessor = pstd_type_model_get_accessor(type_model, color, "red");
```

After we have the type accessor, we can read and write the field with the type instance.

```C
pstd_type_instance_t* type_inst = PSTD_TYPE_INSTANCE_LOCAL_NEW(type_model);

double red = PSTD_TYPE_INST_READ_PRIMITIVE(double, type_inst, read_accessor);
```

Obviously each servlet must use defined types, all the type definition is stored with the protocol database.
`protoman` is the utility program to manage the type database. The following example shows how to list all the
types in the system and inspect one of the type.

```
$ protoman -l
graphics/ColorRGB
graphics/ColoredTriangle3D
graphics/FlattenColoredTriangle3D
graphics/Point2D
graphics/Point3D

$ protoman -T graphics/ColorRGB

TypeName: graphics/ColorRGB
     NameSapce:       graphics
     Size:            12
     Padding Size:    8
     Depends:
                      graphics/Vector3f
     Reverse depends:
                      graphics/ColoredTriangle3D
     Memory layout:
                      0x00000000 - 0x0000000c:	<Base-Type> :: {graphics/Vector3f}
           [Alias] => 0x00000000 - 0x00000004:	r -> value[0]
           [Alias] => 0x00000004 - 0x00000008:	g -> value[1]
           [Alias] => 0x00000008 - 0x0000000c:	b -> value[2]

```

We can also use `protoman` program to create, modify or delete a type definition as well.
When we want to either define or modify a type, we need write a protocol type definition file.

If we want to pack our `getline` servlet result into a structured data pack, we need define the following
type and the type name we can use in servlet is `plumber_tutorial/Line`.

```C
/* protocol.ptype */
package plumber_tutorial;

type Line {
	plumber.std.request_local.String content;
	int32 number;
};
```

With the ptype file, we can install/update the type with the following command.

```
$ protoman -u src/protocol.ptype
Compiling type description file src/protocol.ptype
Validating....
Types to update:
	[0]	plumber_tutorial/Line
Do you want to continue? [y/N] y
Operation sucessfully posted
```

## Request Local Object

Imagine we need to transmit a huge string between servlets, it's very expensive to copy them each time the servlet wants to pass it
to another one. In Plumber we can register an object, for example, a string, as the request local object. Each request local object 
has a unique token id so that each exec callback of the servlet for the same external request can share the memory object with the token.

The string is a good example for the requeset local object, instead of passing the entire string via the pipe, the servlet actually passes
the token through the pipe. 

## Use type in Servlet

Just like what we did for the primitive type last time, we can use the accessor to access the field. So that we need to get the accessor 
for all the fields we need to access in init callback.

```C
line = pipe_define("line", PIPE_OUTPUT, "plumber_tutorial/Line");

type_model = pstd_type_model_new();
line_no_acc = pstd_type_model_get_accessor(type_model, line, "number");
line_acc = pstd_type_model_get_accessor(type_model, line, "content.token");
```

For the exec callback, we use the accessor to write the result

```C
scope_token_t token = pstd_string_commit(str_obj);

PSTD_TYPE_INST_WRITE_PRIMITIVE(inst, line_acc, token);
PSTD_TYPE_INST_WRITE_PRIMITIVE(inst, line_no_acc, state->line_num);
```
