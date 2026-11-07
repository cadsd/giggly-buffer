# Input Buffer GUI

Geode mod: input buffer + ImGui menu, toggled with F7. No logo / branding.

## Build
1. Install the Geode SDK + CLI, set `GEODE_SDK`.
2. Download `CPM.cmake` into `cmake/CPM.cmake`.
3. `geode build`

## Notes
- The delay is a plain fixed delay. The original mod's trajectory prediction
  (ghost simulation, orbs/pads/portals, per-mode logic) is NOT included.
- Defaults to practice mode only.
- If `dispatchKeyboardMSG` fails to compile, your bindings use a different
  signature (e.g. without `double timestamp`); adjust to match.
