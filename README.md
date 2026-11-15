# KadRunner

A fast, Ulauncher-inspired Qt 6 / QML frontend for KDE Plasma 6's KRunner (`KF6::Runner`), built for Wayland with `LayerShellQt` and `KGlobalAccel`.

## Features

- **Full KRunner backend:** Uses `KRunner::ResultsModel` directly, supporting all your enabled KRunner plugins (applications, system settings, calculator, shell commands, files, bookmarks, web shortcuts, etc.).
- **Ulauncher-style UI:** Large `48×48` icons, crisp typography, top-aligned result names, bottom-aligned result types/subtext, and `Alt+1`–`Alt+9` quick-launch shortcuts.
- **Frequency / Recency history:** Shows your most often or most recently launched items when the search bar is empty (persisted in `~/.local/share/kadrunner/history.json`).
- **Inline preferences:** Click the subtle gear icon in the search bar to switch default empty-state mode (`Most Often`, `Most Recent`, `Off`) and item count (`1`–`10`).
- **Native Wayland daemon:** Runs persistently in the background (`--daemon`), spawns on the active monitor via `LayerShellQt`, and binds `Meta` / `Meta+/` natively through `KGlobalAccel`.

## Dependencies (Arch Linux)

- `cmake`, `ninja`, `gcc`
- `qt6-base`, `qt6-declarative`
- `krunner`, `kconfig`, `kcoreaddons`, `kglobalaccel`, `kirigami`
- `layer-shell-qt`

## Build & Install

```bash
cmake -S . -B build -G Ninja -DCMAKE_INSTALL_PREFIX="$HOME/.local"
cmake --build build
cmake --install build
```

## Run as a Systemd User Service

```bash
systemctl --user daemon-reload
systemctl --user enable --now kadrunner.service
```

## Controls

| Shortcut | Action |
| --- | --- |
| `Meta` / `Meta+/` | Toggle KadRunner (customizable in *System Settings → Keyboard → Shortcuts → KadRunner*) |
| `Up` / `Down` or `Ctrl+P` / `Ctrl+N` | Navigate results |
| `Enter` | Launch selected item |
| `Alt+1` – `Alt+9` | Launch item directly |
| `Esc` | Close settings / hide KadRunner |
