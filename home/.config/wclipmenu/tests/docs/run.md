# Running the test suite

## Requirements

- A live Wayland session (the paste/roundtrip tests exercise the Wayland clipboard).
- kaprica installed so `kapd` and `kapc` are on PATH (default: /usr/local/bin).
- ImageMagick installed (sixel path).
- NO system kapd running while the suites run: kapd is a singleton, so an active
  system daemon would win the socket and the suites' `-D /tmp/...` dbs would never be
  served (the suites retry, but contention makes them slow/fragile).

## Build + run

```
make clean && make && make test
```

- Builds 6 binaries in bin/ with `-std=c11 -Wall -Wextra -O2 -g` (zero warnings).
- Runs every suite sequentially; any failure -> non-zero exit (make test aborts on the
  first failing suite).
- Each suite spawns its OWN kapd against an isolated db under
  `/tmp/kaprica-tests-<pid>/history.db` (never the real ~/.local/share/kaprica/history.db).
- Expected results:
  test_daemon 23/23, test_text 9/9, test_search 27/27, test_reverse 11/11,
  test_image 16/16, test_picker 34/34.

## Isolated db + ownership

- Each suite calls kt_scratch_dir() to create its db dir, then spawns kapd via
  fork/execv(`/usr/local/bin/kapd`, `-D <db>`).
- Ownership is proven by probing the OWN db with `kapc search -D <db> -l 1` before
  relying on it (kapd_serves_own_db() pattern) - guards against losing the singleton slot.
- The suites kill their kapd in suite_fini.

## Hermeticity

- The real history.db is never opened: `make test` leaves its sha256 unchanged and
  leaves zero kapd/kapc orphans (verified in the F2 gate).

## Per-suite debugging

```
./bin/test_image      # run a single suite (it re-spawns its own kapd)
```

- Suite logs use [ PASS ]/[ RUN ] markers with src:line for every assert.
- kapc `copy` forks a clipboard-owner grandchild that survives the parent; expect
  exit status 0 or 137 - the harness kills it at exit.
