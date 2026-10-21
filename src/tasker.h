/*
 * TASKER is an utility to scan through the files on a repository, identify TODO
 * comments and create issues on github for tracking them.
 *
 * For example, a comment like this:
 *
 *  // TODO: example todo
 * would turn into this:
 *  // TODO(#12345): example todo
 *
 * while keeping additional todo parenthesis data:
 *
 * // TODO(some disclaimer): example todo
 *
 * turns into:
 *
 * // TODO(#12345 some disclaimer): example todo
 * */

#if 0
gcc -x c "$0" -DSHELL_MODE -o /tmp/script && exec /tmp/script "$@"
exit 0
#endif

#include <stdio.h>

#ifndef TASKER_H
#define TASKER_H

#endif

// ---------- IMPLEMENTATION ----------

#define SHELL_MODE // FIXME: remove this
#ifdef SHELL_MODE
int main(void) {
	printf("hello world\n");
	return 0;
}
#endif
