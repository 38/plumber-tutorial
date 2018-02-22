# Advanced PSS Topics

[Prev](https://github.com/38/plumber-tutorial/blob/10-reusable-servlet-and-standard-servlets/README.md) | 
[Tables of Content](https://github.com/38/plumber-tutorial/blob/master/README.md) | 

*Graph Manipulation & Logic Servlets*

*See [diff](https://github.com/38/plumber-tutorial/compare/10-reusable-servlet-and-standard-servlets...11-graph-generator-src) since last chapter*

---

The last example shows how we use standard servlets to make the logic a little bit complicated. But it's a huge challenge when our socket server gets larger and larger. This time we are going to expand the socket server to add more customized greetings. 

It's really complicated when we have four or five different name that needs to be greeted differently. This time we will explorer the PSS features that allows developer build really large pipe binding graph.

## Logic Servlets

The very first idea is can we bind few different servlet with well-defined input and output ports and use it just like a normal servlet ? Also how can we reuse some very common servlet combination ? 

For example, for a lot of application it may validate the path first and read some file from disk, we can define servlet like following.

```javascript
read_and_validate = {
  validator := "path_validator";
  reader    := "file_reader";
  (path) -> "path" validator "output" -> "path" reader "output" -> (output);
}
```

And then we can use `read_and_validate` as a normal servlet with a input port named `path` and an output port named `output`.
After defining a logic servlet, we will be use it multiple times even in the same binding graph. 

## Graph Manipulation

Another useful feature is PSS is a Turing-completed programming language with binding graph as first-class-citizen. 
This means we can dynamically generate either logic servlets or the entire binging graph based on what we need. 

Back to the question from the begining of the chapter, how can we expand our socket server to have multiple custuomized greeting message? Actually we can create a function that generates the greeting logic servlet based on what customized greeting message we want. In the example we used basically two functions for the logic servlet generation: `Service.add_node(graph, name, node)` and `Service.add_pipe(graph, from_node, from_port, to_port, to_node);`. The first function is equivalent to `name := node` in the binding graph literal and the second function is equivalent to `from_node "from_port" -> "to_port" to_node`. And the following code is how we archieve this.

```javascript
var create_greeter = function (word_map, default) {
	var n_names = len(word_map);
	var greeter = {
		namedup  := "dataflow/dup 2";
		selector := "dataflow/demux " + List.reduce(function(x,y){return x + " " + y;}, Dict.keys(word_map));
		extract_name := "dataflow/extract content";
		say_default := "say " + default;
		merger := "dataflow/firstnonempty " + (n_names + 1);
		(name) -> "in" namedup {
			"out0" -> "input" extract_name "output" -> "cond";
			"out1" -> "data";
		} selector "default" -> "name" say_default "out" -> "in0" merger "out" -> (message);
	};

	var id = 0;

	for(var name in word_map)
	{
		var node = "say_" + (id + 1);
		var word = word_map[name];
		Service.add_node(greeter, node, "say " + word);
		Service.add_pipe(greeter, "selector", "out" + id, "name", node);
		Service.add_pipe(greeter, node, "out", "in" + (id + 1), "merger");
		id ++;
	}

	return greeter;
}
```

Then we put the logic greeter to our application.

```javascript
Service.start({
	read_name := "getline";
	greeter   := create_greeter(
		{
			"Plumber": "Hello",
			"Alice"  : "Hey",
			"Bob"    : "Hi"
		},
		@"Greeting\ From\ Plumber"
	);
	() -> "in" read_name "line" -> "name" greeter "message" -> ();
});
```
