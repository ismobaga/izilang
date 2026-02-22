# Multi-Window Strategy for the `ui` Module (raylib backend)

## Decision Summary

**Status**: Accepted — implemented in v0.1  
**Backend**: raylib (single-window)

---

## Background

raylib uses a single, global window state (`InitWindow` / `CloseWindow` /
`BeginDrawing` / `EndDrawing`).  True multi-window support (multiple OS-level
windows in the same process) is not available with the standard raylib API and
is explicitly a **non-goal** for v0.1.

---

## v0.1 Strategy: Single Window + Panels

Instead of multiple OS windows, IZI exposes **panels** — virtual scissor
regions inside a single raylib window.  Each panel has its own local coordinate
space and optional scissor clipping via `BeginScissorMode` / `EndScissorMode`.

### Core Types

| Type | Description |
|------|-------------|
| `Window` | The single OS window created by `ui.createWindow()` |
| `Panel`  | A rectangular sub-region of the window with local coordinates |

### API

```izi
import ui

win = ui.createWindow("Editor", 1200, 800)

left  = win.createPanel(0,   0, 600, 800)
right = win.createPanel(600, 0, 600, 800)

while win.isOpen() {
  win.beginDrawing()
  win.clear(ui.color(20, 20, 24))

  left.begin()
  left.fillRect(0, 0, 600, 800, ui.color(30, 30, 40))
  left.drawText(20, 20, "Left panel", 20, ui.color(240, 240, 240))
  left.end()

  right.begin()
  right.drawText(20, 20, "Right panel", 20, ui.color(240, 240, 240))
  right.end()

  win.endDrawing()
}
```

### `Window` methods

| Method | Signature | Description |
|--------|-----------|-------------|
| `createPanel` | `(x, y, width, height) -> Panel` | Create a panel at the given window-space rectangle |
| `isOpen` | `() -> bool` | True while the window has not been closed |
| `beginDrawing` | `()` | Begin a render frame |
| `endDrawing` | `()` | End a render frame and present |
| `clear` | `(color)` | Clear the window background |
| `setTitle` | `(title)` | Change the window title |
| `getSize` | `() -> {width, height}` | Query current window dimensions |
| `getFps` | `() -> number` | Current frames-per-second |
| `setTargetFPS` | `(fps)` | Set the target frames-per-second cap |
| `getFrameTime` | `() -> number` | Seconds elapsed during the last frame (delta time) |
| `setWindowMinSize` | `(width, height)` | Set the minimum resizable window dimensions |
| `toggleFullscreen` | `()` | Toggle between windowed and fullscreen mode |
| `isWindowFocused` | `() -> bool` | True when the window has input focus |
| `measureText` | `(text, fontSize) -> number` | Return the pixel width of a text string |
| `drawText` | `(x, y, text, fontSize, color)` | Draw text at window coordinates |
| `fillRect` | `(x, y, w, h, color)` | Draw a filled rectangle at window coordinates |
| `drawRect` | `(x, y, w, h, color)` | Draw a rectangle outline at window coordinates |
| `fillRectRounded` | `(x, y, w, h, roundness, color)` | Draw a filled rounded rectangle (roundness 0–1) |
| `drawRectRounded` | `(x, y, w, h, roundness, color)` | Draw a rounded rectangle outline (roundness 0–1) |
| `drawLine` | `(x1, y1, x2, y2, thickness, color)` | Draw a line at window coordinates |
| `drawCircle` | `(x, y, radius, color)` | Draw a filled circle at window coordinates |
| `drawCircleLines` | `(x, y, radius, color)` | Draw a circle outline at window coordinates |
| `close` | `()` | Close the window |

### `Panel` methods

All coordinate arguments are **panel-local** (relative to the panel's top-left
corner).  Drawing calls are translated to window space before being forwarded
to raylib.  `panel.begin()` activates a scissor rectangle that clips rendering
to the panel bounds; `panel.end()` removes it.

| Method | Signature | Description |
|--------|-----------|-------------|
| `begin` | `()` | Activate scissor clipping for this panel |
| `end` | `()` | Deactivate scissor clipping |
| `getMousePosition` | `() -> {x, y}` | Mouse position in panel-local coordinates |
| `containsMouse` | `() -> bool` | True when the mouse cursor is inside this panel |
| `measureText` | `(text, fontSize) -> number` | Return the pixel width of a text string |
| `drawText` | `(x, y, text, fontSize, color)` | Draw text in panel-local coordinates |
| `fillRect` | `(x, y, w, h, color)` | Draw a filled rectangle in panel-local coordinates |
| `drawRect` | `(x, y, w, h, color)` | Draw a rectangle outline in panel-local coordinates |
| `fillRectRounded` | `(x, y, w, h, roundness, color)` | Draw a filled rounded rectangle (roundness 0–1) |
| `drawRectRounded` | `(x, y, w, h, roundness, color)` | Draw a rounded rectangle outline (roundness 0–1) |
| `drawLine` | `(x1, y1, x2, y2, thickness, color)` | Draw a line in panel-local coordinates |
| `drawCircle` | `(x, y, radius, color)` | Draw a filled circle in panel-local coordinates |
| `drawCircleLines` | `(x, y, radius, color)` | Draw a circle outline in panel-local coordinates |

### Implementation Notes (raylib backend)

- `panel.begin()` calls `BeginScissorMode(x, y, w, h)`.
- `panel.end()` calls `EndScissorMode()`.
- Drawing methods translate local `(x, y)` to window space by adding the
  panel's origin before forwarding the call to the corresponding raylib function.
- **Nested panels are not supported in v0.1.**  Call `panel.end()` before
  beginning another panel to avoid undefined scissor-mode nesting.
- When two panels overlap, the last panel whose `begin()` was called determines
  the active clipping rectangle (last-writer-wins).

---

## Alternative Options (documented; not for v0.1)

### Option A — Multi-process windows

`ui.spawnWindow(scriptPath)` launches a new IZI interpreter process.  Each
process owns one raylib window.

- ✅ Real OS windows, simple isolation  
- ❌ Requires IPC for data sharing between windows

### Option B — Backend abstraction (future)

Define a C++ interface (`IUiBackend`) with methods such as
`createWindow`, `pollEvents`, `beginDrawing`, etc.  The raylib backend
implements single-window semantics; an SDL3 or GLFW backend could implement
true multi-window.

---

## Constraints / Risks

- **Single window per process**: `ui.createWindow()` enforces this at runtime
  when built with raylib.
- **Scissor nesting**: v0.1 does not support nested panel scissor regions.
  Panels must be used sequentially (`left.begin()` … `left.end()` then
  `right.begin()` … `right.end()`).
- **API names are backend-neutral** so the same IZI source code will remain
  compatible with future backends that support true multi-window.

---

## Acceptance Criteria (v0.1)

- [x] Documentation updated: raylib backend is single-window
- [x] Implement `Window.createPanel(x, y, w, h) -> Panel`
- [x] Implement scissor clipping per panel (`Panel.begin()` / `Panel.end()`)
- [x] Implement panel-relative mouse utilities:
  - [x] `panel.getMousePosition() -> {x, y}`
  - [x] `panel.containsMouse() -> bool`
- [x] Add minimal sample demonstrating 2 panels in one window (`examples/ui_panels.izi`)
- [x] Tests validate panel API (creation, method presence, coordinate helpers)

## Additions (UI Enhancement)

- [x] `win.measureText(text, fontSize) -> number` — pixel width of text for layout
- [x] `win.drawCircleLines(x, y, radius, color)` — circle outline
- [x] `win.fillRectRounded(x, y, w, h, roundness, color)` — filled rounded rectangle
- [x] `win.drawRectRounded(x, y, w, h, roundness, color)` — rounded rectangle outline
- [x] `win.toggleFullscreen()` — toggle windowed/fullscreen mode
- [x] `win.isWindowFocused() -> bool` — check window focus
- [x] `win.setWindowMinSize(width, height)` — enforce a minimum window size
- [x] `panel.measureText(text, fontSize) -> number` — panel-level text measurement
- [x] `panel.drawCircleLines(x, y, radius, color)` — panel-level circle outline
- [x] `panel.fillRectRounded(...)` / `panel.drawRectRounded(...)` — rounded rects in panels
- [x] `ui.getTime() -> number` — seconds elapsed since window initialization
- [x] Expanded `ui.key` constants: full alphabet (a–z), F1–F12, backspace, tab, delete, shift, ctrl, alt
- [x] Fixed `win.close()` double-assignment of `win->open = false`
