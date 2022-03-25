# build tutorial

based on: https://docs.bazel.build/versions/4.2.2/tutorial/cpp.html

### Build with Bazel

- The `WORKSPACE` file, which identifies the directory and its contents as a Bazel workspace and lives at the root of the project's directory structure.
- One or more `BUILD` files, which tell Bazel how to build different parts of the project. (A directory within the workspace that contains a `BUILD` file is a package. You will learn about packages later in this tutorial.)

### Understand the BUILD file

A `BUILD` file contains several different types of instructions for Bazel. The most important type is the _build rule_, which tells Bazel how to build the desired outputs, such as executable binaries or libraries. Each instance of a build rule in the `BUILD` file is called a _target_ and points to a specific set of source files and dependencies. A target can also point to other targets.

In our example, the `hello-world` target instantiates Bazel's built-in `cc_binary` rule. The rule tells Bazel to build a self-contained executable binary from the `hello-world.cc` source file with no dependencies. 

The attributes in the target explicitly state its dependencies and options. while the `name` attribute is mandatory, many are optional. For example, in the `hello-world` target, `name` is required qnd self-explanatory, and `srcs` is optional and specifies the source files from which Bazel builds the target. 

### Build the project

To build your sample project, navigate to the `cpp-tutorial/stage1` directory and run:

```
bazel build //main:hello-world
```

In the target label, the `//main:` part is the location of the `BUILD` file relative to the root of the workspace, and `hello-world` is the target name in the `BUILD` file. 

Bazel places build outputs in the `bazel-bin` directory at the root of the workspace. Browse through its contents to get an idea for Bazel's output structure.

Test your freshly build binary:

```
bazel-bin/maim/hello-world
```

### Review the dependency graph

A succesful build has all of its dependencies explicitly stated in the `BUILD` file. Bazel uses those statements to create the project's dependency graph, which enables accurate incremental builds. 

To visualize:

```
bazel query --notool_deps --noimplicit_deps "deps(//main:hello-world)" \
  --output graph
```

### Refine your Bazel build

You can split the sample project build into two targets. 

```
cc_library(
    name = "hello-greet",
    srcs = ["hello-greet.cc"],
    hdrs = ["hello-greet.h"],
)

cc_binary(
    name = "hello-world",
    srcs = ["hello-world.cc"],
    deps = [
        ":hello-greet",
    ],
)
```