# cutils

A collection of single file utilities and libraries for C. Each utility is a single
header file that you can either execute as a standalone unix shell aplication, or
use it in your project as a C library (stb style).

## Using a header

As a library, define the implementation macro in exactly one translation unit (probably your main.c).

Example with tasker:

```c
#define TASKER_IMPL
#include "tasker.h"
```

As a tool, the file compiles itself through the `#if 0` block at the top. That block
is a simple hack to allow you to execute the file directly with `./file.h`. It compiles
the file as an executable in your /tmp folder and runs it, passing all the relevant
shell flags

```sh
chmod +x ./src/tasker.h
./src/tasker.h
```

It passes `-DSHELL_MODE` to the compiler, which is what gates `main`.

## tasker

Walks the repo looking for TODO comments, opens one issue per TODO on github and
writes the issue number back into the comment.

```c
// TODO: handle empty input
```

becomes

```c
// TODO(#42): handle empty input
```

Anything already inside the parenthesis is preserved, and TODOs that already
have an issue are left alone, so running it again is a no-op:

```c
// TODO(needs design): retry logic  ->  // TODO(#42 needs design): retry logic
// TODO(#7): already tracked        ->  unchanged
```

Github interface is done with `gh` cli, so there are no tokens to manage.

## Conventions

- C99, single header. `#define TASKER_IMPL` in one translation unit before including.
- No dependencies besides libc.
- Unity build friendly.
