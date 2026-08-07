/*
 * wclipmenu — wmenu-based clipboard picker over kaprica (Wayland).
 *
 * Runs on demand from a keybind and exits after one selection:
 *   wclipmenu            -> pick from kaprica text history via wmenu, copy choice
 *   wclipmenu list       -> print snippet lines to stdout (debug/CLI path)
 *   wclipmenu copy <id>  -> kapc copy -i <id> (debug/CLI path)
 *   wclipmenu image      -> pick from kaprica image/png history via wmenu
 *                           (PNG thumbnails), copy choice
 *
 * Zero resident footprint: no daemon, no polling, no config files.
 * Depends only on libc + external wmenu + kaprica CLI (kapd/kapc).
 *
 * Env overrides (for testing):
 *   WCLIPMENU_WMENU  path to wmenu binary (default: wmenu from PATH)
 *   WCLIPMENU_LIMIT  max entries from kaprica (default: 100)
 *   WCLIPMENU_DB     db file path passed to kapc as -D (default: kaprica's)
 */
#ifndef WCLIPMENU_H
#define WCLIPMENU_H

#include <stddef.h> /* size_t */

#define KAPC_PATH "/usr/local/bin/kapc"
#define DEFAULT_LIMIT 100
#define WMENU_LINES 15
#define WMENU_IMAGE_LINES 5
#define WMENU_PROMPT "clipboard:"

struct entry {
	long id;
	const char *snippet; /* pointer into the search buffer */
};

/* kapc.c */
int run_capture_raw(const char *const argv[], char **out, size_t *outlen);
int run_capture(const char *const argv[], char **out);
int spawn_detach(const char *const argv[]);
int pipeline_detach(const char *const a[], const char *const b[]);
int run_quiet(const char *const argv[]);
int kapc_search_type(const char *type, int limit, char **out);
int kapc_search(int limit, char **out);
char *kapc_copy_cmd(long id);

/* entries.c */
size_t parse_entries(char *raw, struct entry *es, size_t max);

/* wmenu.c */
int run_wmenu_lines(const char *input, int wmenu_lines, char **sel);
int run_wmenu(const struct entry *es, size_t n, char **sel);

/* commands.c */
int cmd_list(int limit);
int cmd_copy(long id);
int cmd_copy_image(long id);
int cmd_pick(int limit);

/* image.c */
int cmd_image(int limit);

#endif /* WCLIPMENU_H */
