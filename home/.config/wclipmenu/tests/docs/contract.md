# Kaprica CLI contract (observed + source-verified)

Verified against kaprica source at ~/builds/kaprica (src/kaprica.c, src/database.c) and
locked by the test suites in this repo. "Locked" = asserted in a passing suite.

## Daemon (kapd)

- Singleton per system: at most one instance serves the Wayland clipboard socket at a
  time. A second instance started while one is active may start but does NOT bind, so
  its `-D` database path is not served (source: no rebind path; observed + locked by
  test_daemon refusal test).
- `-D <path>` selects the sqlite history database (default: ~/.local/share/kaprica/history.db).
- Because of the singleton, a suite must spawn its own kapd, then PROVE ownership by
  querying its own db (`kapc search -D g_db -l 1`), retrying the spawn on contention
  (locked: test_daemon 23/23, test_image ownership probe).
- Handles SIGINT/SIGTERM via signalfd for clean cleanup (source: kapricad.c:260).
- Never runs two kapd instances with the same db; never point it at the real user db.

## CLI parsing (kapc)

- Options come AFTER the subcommand: `kapc search -D db -l 1`.
  `kapc -D db search` is a usage error (exit 1). (source: getopt_long(argc-1, argv+1),
  kaprica.c:254; locked: refusal tests.)
- Subcommands observed: `copy`, `paste`, `search`. Global flags: `-D` (db), `-l` (limit),
  `-i` (entry id), `-t` (mime type), `-L` (list newest), `-x` (sixel), `-r` (reverse-search).

## Text

- `kapc copy` reads the payload from stdin; empty stdin -> "Nothing to copy" (stderr).
- `-t <mime>` pins the mime type (source: kaprica.c:682-684); otherwise the type is
  guessed via guess_mime_types() (source: kaprica.c:688).
- `kapc copy` forks a clipboard-owner child that must stay alive to serve the Wayland
  selection; do NOT assert on its exit status (0 or 137 are both valid). (locked:
  text/copy suites.)
- `kapc search` default line format: `ID: <n>\t"<snippet>"` — snippet newline-trimmed.
- `kapc paste -i <id>` writes the stored bytes to stdout (roundtrip byte-exact; locked).

## Search

- `-L` lists NEWEST-FIRST: SQL `ORDER BY timestamp DESC LIMIT ?1 OFFSET ?2`
  (source: database.c:157; locked: test_search asserts oldest seed is excluded).
- `-l <n>` limits results.
- `-t <mime>` filters by mime (e.g. `-t text/plain`, `-t image/png`).
- `-r, --reverse-search` takes a snippet on stdin, finds the entry in the db
  (database_find_entry_from_snippet), and copies it (source: kaprica.c:654-668).

## Image / sixel

- `kapc copy -t image/png < file.png` stores an image entry (stored as image/png).
- `kapc search -x` emits a sixel image (DCS sequence starting `ESC P` = 0x1b 0x50)
  rendered from the stored thumbnail via ImageMagick (thumbnail_to_sixel).
- If sixel generation fails (e.g. undecodable image), sixel_len == 0 and output FALLS
  BACK to the text line format (source: kaprica.c:849 `if (options.sixel && sixel_len > 0)`;
  observed: "IDAT: incorrect header check" from ImageMagick -> text fallback; locked:
  test_image sixel + roundtrip suites).
- Sixel output is only produced for image entries; `-x` on a text entry prints the text
  line format (observed).
- ImageMagick is a hard runtime dependency of the sixel path; a PNG whose IDAT lacks the
  2-byte zlib header (CMF/FLG 0x78 0x01) is stored fine but fails sixel decoding.

## Runtime requirements

- Live Wayland session (clipboard offer/paste paths).
- kapd + kapc installed (default /usr/local/bin/kapd, /usr/local/bin/kapc).
- No system kapd running when a suite spawns its own (singleton above).
