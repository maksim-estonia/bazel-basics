- [Bazel docs](#bazel-docs)
  - [Concepts](#concepts)
    - [Workspaces, packages & targets](#workspaces-packages--targets)
      - [Workspace](#workspace)
      - [Packages](#packages)
      - [Targets](#targets)
    - [Labels](#labels)
      - [Rules](#rules)
    - [BUILD files](#build-files)
    - [Dependencies](#dependencies)
    - [Visibility](#visibility)
    - [Platforms](#platforms)
    - [Hermeticity](#hermeticity)
  - [Work with BUILD files](#work-with-build-files)
    - [BUILD style guide](#build-style-guide)
    - [Share variables](#share-variables)
    - [Work with external dependencies](#work-with-external-dependencies)
    - [Manage dependencies with Bzlmod](#manage-dependencies-with-bzlmod)
  - [Run Bazel](#run-bazel)
    - [Build with Bazel](#build-with-bazel)
    - [Commands and options](#commands-and-options)
    - [Write bazelrc files](#write-bazelrc-files)
    - [Call Bazel from scripts](#call-bazel-from-scripts)
    - [Client/server implementation](#clientserver-implementation)
  - [Configure your build](#configure-your-build)
    - [Configurable attributes](#configurable-attributes)
    - [Integrate with C++ rules](#integrate-with-c-rules)
    - [Toolchain resolution implementation](#toolchain-resolution-implementation)
    - [Code coverage with Bazel](#code-coverage-with-bazel)
  - [Query your build](#query-your-build)
    - [Query quickstart](#query-quickstart)
    - [Query reference](#query-reference)
    - [Action graph query (aquery)](#action-graph-query-aquery)
    - [Configurable query (cquery)](#configurable-query-cquery)
  - [Optimizing Bazel](#optimizing-bazel)
    - [Best practices](#best-practices)
    - [Optimize memory](#optimize-memory)

# Bazel docs

## Concepts

### Workspaces, packages & targets

Source files in the workspace are organized in a nested hierarchy of packages, where each package is a directory that contains a set of related source files and one `BUILD` file. The `BUILD` file specifies what software outputs can be built from the source.

#### Workspace

A workspace is a directory tree on your filesystem that contains the source files for the software you want to build. Each workspace has a text file named `WORKSPACE` which may be empty, or may contain references to external dependencies required to build the outputs.

Directories containing a file called `WORKSPACE` are considered the root of a workspace. Therefore, Bazel ignores any directory trees in a workspace rooted at a subdirectory containing a `WORKSPACE` file, as they form another workspace.

Bazel also supports `WORKSPACE.bazel` file as an alias of `WORKSPACE` file. If both files exist, `WORKSPACE.bazel` is used.

Code is organized in _repositories_. The directory containing the `WORKSPACE` file is the root of the main repository, also called `@`. Other, (external) repositories are defined in the `WORKSPACE` file using workspace rules.

As external repositories are repositories themselves, they often contain a `WORKSPACE` file as well. However, these additional `WORKSPACE` files are ignored by Bazel. 

#### Packages

The primary unit of code organization in a repository is the _package_. A package is a collection of related files and a specification of how they can be used to produce output artifacts. 

A package is defined as a directory containing a file named `BUILD` (or `BUILD.bazel`). A package includes all files in its directory, plus all subdirectories beneath it, except those which themselves contain a `BUILD` file. From this definition, no file or directory can be a part of two different packages simultanously.

#### Targets

A package is a container of _targets_, which are defined in the package's `BUILD` file. Most targets are one of two principal kinds, _files_ and _rules_.

Files are further divided into two kinds. _Source files_ are usually written by the efforts of people, and checked in to the repository. _Generated files_, sometimes called derived files or output files, are not checked in, but are generated from source files.

The second kind of target is declared with a _rule_. Each rule instance specifies the relationship between a set of input and a set of output files. The inputs to a rule may be source files, but they also may be the outputs of other rules.

Whether the input to a rule is a source file or a generated file is in most cases irrelevant; what matters is only the contents of that file. This fact makes it easy to replace the complex source file with a generated file produced by a rule, such as happens when the burden of manually maintaining a highly structured file becomes too tiresome, and someone writes a program to derive it. No change is required to the consumers of that file. Conversely, a generated file may easily be replaced by a source file with only local changes.

The inputs to a rule may also include _other rules_. The precise meaning of such relationships is often quite complex and language- or rule-dependent, but intuitively it is simple: a C++ library rule A might have another C++ library rule B for an input. The effect of this dependency is that B's header files are available to A during compilation, B's symbols are available to A during linking, and B's runtime data is available to A during execution.

An invariant of all rules is that the files generated by a rule always belong to the same package as the rule itself; it is not possible to generate files into another package. It is not uncommon for a rule's input to come from another package, though.

Package groups are sets of packages whose purposes is to limit accessibility of certain rules. Package groups are defined by the `package_group` function. They have three properties: the list of packages they contain, their name, and other package groups they include. The only allowed ways to refer to them are form the `visibility` attribute of rules of from the `default_visibility` attribute of the `package` function; they do not gerenate or consume files.

### Labels

All targets belong to exactly one package. The name of a target is called its label. Every label uniquely identifies a target. A typical label in canonical form looks like:

```
@myrepo//my/app/main:app_binary
```

The first part of the label is the repository name, `@myrepo//`. In the typical case that a label refers to the same repository from which it is used, the repository identifier may be abbreviated as `//`. So, inside `@myrepo` this label is usually written as

```
//my/app/main:app_binary
```

The second part of the lable is the un-qualified package name `my/app/main`, the path to the package relative to the repository root. Together, the repository name and the un-qualified package name form the fully-qualified package name. When the label refers to the same package it is used in, the package name (and optionally, the colon) may be omitted. So, inside `@myrepo//my/app/main`, this label may be written either of the following ways:

```
app_binary
:app_binary
```

It is a matter of convention that the colon is omitted for files, but retained for rules, but it is not otherwise significant.

The part of the label after the colon, `app_binary` is the un-qualified target name. When it matches the last component of the package path, it and the colon, may be omitted. So, these two labels are equivalent:

```
//my/app/lib
//my/app/lib:lib
```

The name of a file target in a subdirectory of the package is the file's path relative to the package root (the directory containing the `BUILD` file). So, this file is in the `my/app/main/testdata` subdirectory of the repository:

```
//my/app/main:testdata/input.txt
```

Don't confuse labels like //my/app with package names. Labels always start with a repository identifier (often abbreviated `//`), but package names never do. Thus, `my/app` is the package containing `//my/app/lib` (which can also be written as `//my/app/lib:lib`).

Labels starting with `@//` are references to the main repository, which will still work even from external repositories. Therefore `@//a/b/c` is different from `//a/b/c` when referenced from an external repository. The former refers back to the main repository, while the latter looks for `//a/b/b` in the external repository itself. This is especially relevant when writing rules in the main repository that refers to targets in the main repository, and will be used from external repositories.

**Lexical specification of a label**

Label syntax discourages use of metacharacters that have special meaning to the shell. This helps to avoid inadvertent quoting problems, and makes it easier to construct tools and scripts that manipulate labels.

The precise details of allowed target names are below.

- Target names - package-name:target-name

`target-name` is the name of the target within the package. The name of a rule is the value of the `name` attribute in the rule's declaration in a `BUILD` file; the name of a file is its pathname relative to the directory containing the `BUILD` file.

- Package name - //package-name:target-name

The name of a package is the name of the directory containing its `BUILD` file, relative to the top-level directory of the containing repository. For example: `my/app`

#### Rules

A rule specifies the relationship between inputs and outputs, and the steps to build the outputs. Rules can be of one of many different kinds (sometimes called the _rule class_), which produce compiled executables and libraries, test executables and other supported outputs. 

`BUILD` files declare _targets_ by invoking _rules_.

In the example below, we see the declaration of the target `my_app` using the `cc_binary` rule. 

```
cc_binary(
    name = "my_app",
    srcs = ["my_app.cc"],
    deps = [
        "//absl/base",
        "//absl/strings",
    ],
)
```

Every rule invocation has a `name` attribute (which must be a valid target name), declares a target within the package of the `BUILD` file. 

Every rule has a set of _attributes_; the applicable attributes for a given rule, and the significance and semantics of each attribute are a function of the rule's kind. 

In other cases, the name is significant for `*_binary` and `*_test` rules, for example, the rule name determines the name of the executable produced by the build.

This directed graph over targets is called the _target graph_ or `build dependency graph`, and is the domain over which the Bazel Query tool operates.

### BUILD files

The previous sections describes packages, targets and labels, and the build dependency graph abstractly. This section describes the concrete syntax used to define a package.

By definition, every package contains a `BUILD` file, which is a short program. `BUILD` files are evaluated using an imperative language, Starlark.

They are interpreted as a sequential list of statements. 

In general, order does matter: variables must be defined before they are used, for example. However, most `BUILD` files consist only of declarations of build rules, and the relative order of these statements is immaterial; all that matters is which rules were declared, and with what values, by the time package evaluation completes.

When a build rule function, such as `cc_library`, is executed, it creates a new target in the graph. This target can later be referred using a label.

In simple `BUILD` files, rule declarations can be re-ordered freely without changing the behavior. 

To encourage a clean seperation between code and data, `BUILD` files cannot contain function definitions, `for` statements or `if` statements. 

### Dependencies

### Visibility

### Platforms

### Hermeticity

## Work with BUILD files

### BUILD style guide

### Share variables

### Work with external dependencies

### Manage dependencies with Bzlmod

## Run Bazel

### Build with Bazel

### Commands and options

### Write bazelrc files

### Call Bazel from scripts

### Client/server implementation

## Configure your build

### Configurable attributes

### Integrate with C++ rules

### Toolchain resolution implementation

### Code coverage with Bazel

## Query your build

### Query quickstart

### Query reference

### Action graph query (aquery)

### Configurable query (cquery)

## Optimizing Bazel

### Best practices

### Optimize memory