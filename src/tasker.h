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
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef TASKER_H
#define TASKER_H

#endif

///--------------------------BEGIN ARRAY----------------------------------------
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
#define arr_len(arr)                                                           \
	(((arr) != NULL) ? ((((DA_HEADER *)(arr)) - 1)->count) : (0))
///--------------------------END ARRAY------------------------------------------

///--------------------------BEGIN ARENA----------------------------------------
#define KB(x) (((size_t)x) * (size_t)1024)
#define MB(x) (KB(x) * ((size_t)x))
typedef struct {
	void *memory;
	size_t offset;
	size_t capacity;
} Arena;
Arena *arena_alloc(size_t capacity) {
	void *backing_memory = malloc(capacity + sizeof(Arena));
	Arena *arena = backing_memory;
	arena->memory = (void *)((Arena *)backing_memory + 1);
	arena->capacity = capacity;
	arena->offset = 0;
	return arena;
}
void *arena_push(Arena *arena, size_t bytes) {
	if (arena->offset + bytes >= arena->capacity) {
		size_t new_capacity = arena->capacity * 1.5;

		void *start = ((Arena *)arena->memory) - 1;

		start = realloc(start, new_capacity);
		arena->capacity = new_capacity;
		arena->memory = (void *)(((Arena *)start) + 1);
	}

	void *ptr = arena->memory + arena->offset;
	arena->offset += bytes;
	return ptr;
}
void arena_clear(Arena *arena) { arena->offset = 0; }
void arena_free(Arena *arena) {
	void *header = ((Arena *)arena->memory) - 1;
	free(header);
}
#define push_struct(a, T) (T *)arena_push(a, sizeof(T))
#define push_array(a, T, c) (T *)arena_push(a, sizeof(T) * c)
///--------------------------END ARENA------------------------------------------

///--------------------------BEGIN STR------------------------------------------
typedef struct {
	char *chars;
	size_t len;
	size_t capacity;
	bool is_static;
} String;
String string_from_cstr(Arena *arena, char *cstr);
///--------------------------END STR--------------------------------------------

char **get_all_files(char *dirname, char **files);
void run(char *root);

// ---------- IMPLEMENTATION ----------

#ifdef TASKER_IMPL
String string_from_cstr(Arena *arena, char *cstr) {
	String result = {0};
	result.len = strlen(cstr);
	result.capacity = result.len * 2;
	result.chars = push_array(arena, char, result.capacity);
	memcpy(result.chars, cstr, result.len);
	return result;
}
String String_from_lit(char *lit) {
	size_t len = strlen(lit);
	return (String){
		.chars = lit, .len = len, .capacity = len, .is_static = true};
}

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
	char **all_files = get_all_files(dirname, NULL);

	Arena *line_arena = arena_alloc(MB(1));
	for (size_t i = 0; i < arr_len(all_files); i++) {
		char *filepath = all_files[i];

		FILE *fp = fopen(filepath, "r");
		char linebuf[4096];
		while (fgets(linebuf, 4096, fp) != NULL) {
			String line = string_from_cstr(line_arena, linebuf);

			arena_clear(line_arena);
		}
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
