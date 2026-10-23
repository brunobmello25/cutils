#if 0
gcc -x c "$0" -Wall -Werror -DSHELL_MODE -DTASKER_IMPL -o /tmp/script && exec /tmp/script "$@"
exit 0
#else
// FIXME: remove this
#ifndef SHELL_MODE
#define SHELL_MODE
#endif
#ifndef TASKER_IMPL
#define TASKER_IMPL
#endif
#endif

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

#include <assert.h>
#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef TASKER_H
#define TASKER_H

#endif

typedef struct {
	size_t count;
	size_t capacity;
} DA_HEADER;
#define DA_INITIAL_CAPACITY 256

#define arr_push(arr, value)                                                   \
	do {                                                                       \
		if (arr == NULL) {                                                     \
			void *memory = malloc(DA_INITIAL_CAPACITY * sizeof(*arr) +         \
								  sizeof(DA_HEADER));                          \
			DA_HEADER *header = (DA_HEADER *)memory;                           \
			header->capacity = DA_INITIAL_CAPACITY;                            \
			arr = (void *)(((DA_HEADER *)memory) + 1);                         \
		}                                                                      \
                                                                               \
		DA_HEADER *header = ((DA_HEADER *)arr) - 1;                            \
		if (header->count >= header->capacity) {                               \
			header->capacity *= 2;                                             \
			header = realloc(header, sizeof(*arr) * header->capacity +         \
										 sizeof(DA_HEADER));                   \
		}                                                                      \
		arr = (void *)(header + 1);                                            \
		arr[header->count++] = value;                                          \
	} while (0);

#define arr_len(arr) ((((DA_HEADER *)(arr)) - 1)->count)

char **get_all_files(char *dirname, char **files);
void run(char *root);

// ---------- IMPLEMENTATION ----------

#ifdef TASKER_IMPL
static void fail(const char *msg) {
	fprintf(stderr, "%s", msg);
	exit(1);
}

static char *path_join(const char *left, const char *right) {
	size_t llen = strlen(left);
	size_t rlen = strlen(right);

	char *joined = malloc(llen + 1 + rlen + 1);
	memcpy(joined, left, llen);
	joined[llen] = '/';
	memcpy(joined + llen + 1, right, rlen);
	joined[llen + rlen + 1] = '\0';
	return joined;
}

// TODO(next): get the fullpath of the files instead of just the name, so that
// we can iterate on them individually
char **get_all_files(char *dirname, char **files) {
	DIR *dir = opendir(dirname);
	if (!dir)
		fail("failed to open provided directory");

	struct dirent *e;
	while ((e = readdir(dir)) != NULL) {
		if (e->d_name[0] == '.')
			continue; // "." and ".."
		char *full = path_join(dirname, e->d_name);
		if (e->d_type == DT_DIR) {
			files = get_all_files(full, files);
			free(full);
		} else
			arr_push(files, full);
	}

	closedir(dir);
	return files;
}

void run(char *dirname) {
	printf("received directory %s\n", dirname);
	char **all_files = get_all_files(dirname, NULL);
	printf("Files:\n");
	for (size_t i = 0; i < arr_len(all_files); i++) {
		printf("  - %s\n", all_files[i]);
	}
}
#endif

// ---------- SHELL MODE ----------

#ifdef SHELL_MODE
int main(int argc, char **argv) {
	if (argc != 2) {
		printf("USAGE:\n");
		printf("  ./tasker.h path/to/project/root\n");
		return 1;
	}

	run(argv[1]);
}
#endif
