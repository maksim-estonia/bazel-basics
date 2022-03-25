## Bazel tutorial

### part 1

step 1: install [bazelisk](https://docs.bazel.build/versions/main/install-bazelisk.html)

(i used binary for installing on ubuntu)

step 2: `touch .bazelversion`, specify version

```
4.2.2
```

step 3: `touch WORKSPACE.bazel`, one in each project

leave empty for now

`bazel --version` should print 4.2.2 the version of bazel we are using

`bazel build //...` builds all the targets, however gives error now since we don't have any targets

create `projects` structure, `projectA` might use a different language from `projectB`.

Now we can start building: `bazel build //...`

Build only projectA: `bazel build //projects/projectA/...`

[genrule](https://docs.bazel.build/versions/main/be/general.html#genrule)

A genrule generates one or more files using a user-defined Bash command.

Genrules are generic build rules that you can use if there's no specific rule for the task. For example, you could run a Bash one-liner. If however you need to compile C++ files, stick to the existing `cc_*` rules, because all the heavy lifting has already been done for you.

Do not use genrule for running tests. There are special dispensations for tests and test results, including caching policies and environment variables. Tests generally need to be run after the build is complete and on the target architecture, whereas genrules are executed during the build and on the host architecture (the two may be different). If you need a genereal purpose testing rule, use `sh_test`.

### part 2

new project "calculator"

documentation on this topic: https://docs.bazel.build/versions/main/be/python.html

