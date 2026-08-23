# bar-systray (StatusNotifier system tray)

## Purpose
System tray on the dwl bar using StatusNotifier protocol (DBus). Shows tray icons from apps like nm-applet, pasystray, kdeconnect-indicator, etc.

## Source
- Author: vetu104 (janetski)
- Date: 2026-03-29
- Original: [dwl-patches/bar-systray](https://codeberg.org/dwl/dwl-patches/src/branch/main/patches/bar-systray)
- Target: dwl 0.7 + sewn's bar patch

## Dependencies
- `bar.patch` (sewn's bar) — already applied
- `libdbus` (dbus-1) — pkg-config available

## Changes
- `dbus.c/h` — DBus connection + event loop integration
- `systray/` — tray, watcher, item, icon, menu, helpers (2681 lines)
- `dwl.c` — tray creation in `updatebar()`, compositing in `drawbar()`, click handling (`ClkTray`), cleanup
- `config.def.h` — `systrayspacing`, `showsystray`, `dmenucmd`, `ClkTray` button binds
- `Makefile` — `dbus-1` in PKGS, systray object rules

## Applied
Commit `1ee80fe` — "feat: StatusNotifier system tray on the bar (bar-systray patch)"

## Port notes
- Ported from dwl 0.7 to 0.8-dev (wlroots-0.19)
- Hand-merged 7 rejected hunks in dwl.c (drawbar uses drwl.h, appicons use tag_icons[], geometry publish in updatebar)
- Config synced to both `config.def.h` and live `config.h`

## Usage
- Requires DBus session (`dbus-run-session` or systemd user session)
- Tray apps auto-register via StatusNotifierItem
- Left-click: activate, Right-click: menu (uses `dmenucmd` = wmenu)
