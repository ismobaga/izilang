# ui — Graphical Window and Drawing

The `ui` module provides a graphical window and drawing API backed by [raylib](https://www.raylib.com/). It enables interactive GUI applications directly from IziLang scripts.

> **Note**: The `ui` module requires raylib. Build the interpreter with `--raylib=<path>` passed to `premake5` to enable full functionality. Without raylib, the module can be imported and constants/color values work, but calling window-creation or drawing functions will throw a runtime error.

## Import

```izilang
import ui

// Or with an alias
import * as ui from "std.ui";
```

## Quick Start

```izilang
import ui

var win = ui.createWindow("My App", 800, 600)
while win.isOpen() {
    if ui.keyPressed(ui.key.escape) { win.close() }
    win.beginDrawing()
    win.clear(ui.color(20, 20, 24))
    win.drawText(20, 20, "Hello IZI", 24, ui.color(240, 240, 240))
    win.endDrawing()
}
```

## Window Creation

### `ui.createWindow(title, width, height)`

Opens a new OS window and returns a `Window` object. Only one window can be open at a time.

```izilang
var win = ui.createWindow("My Application", 1280, 720)
```

## Window Methods

| Method | Signature | Description |
|--------|-----------|-------------|
| `isOpen` | `() → bool` | `true` while the window is open |
| `close` | `()` | Close the window |
| `beginDrawing` | `()` | Begin a render frame |
| `endDrawing` | `()` | End the render frame and display it |
| `clear` | `(color)` | Clear the background |
| `setTitle` | `(title)` | Change the window title |
| `getSize` | `() → {width, height}` | Current window dimensions |
| `getFps` | `() → number` | Current frames per second |
| `drawText` | `(x, y, text, fontSize, color)` | Draw text at (x, y) |
| `fillRect` | `(x, y, w, h, color)` | Draw a filled rectangle |
| `drawRect` | `(x, y, w, h, color)` | Draw a rectangle outline |
| `drawLine` | `(x1, y1, x2, y2, thickness, color)` | Draw a line |
| `drawCircle` | `(x, y, radius, color)` | Draw a filled circle |
| `createPanel` | `(x, y, w, h) → Panel` | Create a scissor-clipped sub-region |

## Panel Methods

Panels are virtual sub-regions of the window. Coordinates are **panel-local** (relative to the panel's top-left corner).

| Method | Signature | Description |
|--------|-----------|-------------|
| `begin` | `()` | Activate scissor clipping |
| `end` | `()` | Deactivate scissor clipping |
| `getMousePosition` | `() → {x, y}` | Mouse position in panel-local coords |
| `containsMouse` | `() → bool` | `true` when the cursor is inside this panel |
| `drawText` | `(x, y, text, fontSize, color)` | Draw text |
| `fillRect` | `(x, y, w, h, color)` | Draw a filled rectangle |
| `drawRect` | `(x, y, w, h, color)` | Draw a rectangle outline |
| `drawLine` | `(x1, y1, x2, y2, thickness, color)` | Draw a line |
| `drawCircle` | `(x, y, radius, color)` | Draw a filled circle |

## Colors

### `ui.color(r, g, b)` / `ui.color(r, g, b, a)`

Creates a color value. Channel values are 0–255. Alpha defaults to 255 (fully opaque).

```izilang
var white  = ui.color(255, 255, 255)
var red    = ui.color(255, 0, 0)
var semiTransparent = ui.color(0, 128, 255, 128)
```

## Keyboard Input

### `ui.keyDown(key)`
Returns `true` while `key` is held down.

### `ui.keyPressed(key)`
Returns `true` on the first frame `key` is pressed.

### `ui.getCharPressed()`
Returns the Unicode codepoint of the last typed character, or `0` if none.

**Key Constants** (`ui.key.*`):

| Constant | Key |
|----------|-----|
| `escape` | Escape |
| `enter` | Enter / Return |
| `space` | Space bar |
| `left` | Left arrow |
| `right` | Right arrow |
| `up` | Up arrow |
| `down` | Down arrow |
| `a`–`z` | Letter keys |

## Mouse Input

### `ui.mouseDown(button)` / `ui.mousePressed(button)`
Check mouse button state.

### `ui.getMousePosition()`
Returns `{x, y}` with the current cursor position.

### `ui.getMouseWheelMove()`
Returns the scroll delta for the current frame.

**Mouse button constants** (`ui.mouse.*`): `left`, `right`, `middle`.

## Building with raylib

```bash
premake5 gmake --raylib=/path/to/raylib
make config=debug
./bin/Debug/izi/izi examples/demo_ui.iz
```

On Linux, `GL` and `X11` are linked automatically.

## Complete Example

```izilang
import ui

var win = ui.createWindow("IZI + raylib", 900, 600)

while win.isOpen() {
    if ui.keyPressed(ui.key.escape) { win.close() }

    win.beginDrawing()
    win.clear(ui.color(20, 20, 24))

    var pos = ui.getMousePosition()
    win.fillRect(pos.x - 50, pos.y - 20, 100, 40, ui.color(80, 140, 255))

    win.drawText(20, 20, "Hello IZI", 24, ui.color(240, 240, 240))
    win.drawCircle(450, 300, 40, ui.color(255, 80, 80))

    var fps = win.getFps()
    win.drawText(20, 570, "FPS: " + toString(fps), 14, ui.color(120, 120, 120))

    win.endDrawing()
}
```

## See Also

- [Standard Library Index](README.md)
- [`examples/demo_ui.iz`](../../examples/demo_ui.iz)
- [`examples/ui_panels.izi`](../../examples/ui_panels.izi)
