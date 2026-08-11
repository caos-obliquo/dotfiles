# systemd

User systemd units (started via `systemctl --user`).

## Files

- `user/kaprica.service` — kapd (clipboard watcher daemon), the history
  backend for wclipmenu

## Notes

- Session env is imported by `start-dwl.sh` (`import-environment`), so
  user units inherit `WAYLAND_DISPLAY` etc.
