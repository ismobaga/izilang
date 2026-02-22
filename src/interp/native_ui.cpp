#include "native_ui.hpp"
#include "native.hpp"
#include "interpreter.hpp"
#include <memory>
#include <string>

#ifdef HAVE_RAYLIB
#include <raylib.h>
#endif

namespace izi {

// Internal window state
struct UiWindow {
    bool open = false;
    int width = 0;
    int height = 0;
    std::string title;
};

// Internal panel state (virtual window / scissor region inside a UiWindow)
struct UiPanel {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
};

// ---------------------------------------------------------------------------
// Color helpers
// ---------------------------------------------------------------------------

// Make a color Map value with r, g, b, a fields
static Value makeColorValue(int r, int g, int b, int a = 255) {
    auto m = std::make_shared<Map>();
    m->entries["r"] = static_cast<double>(r);
    m->entries["g"] = static_cast<double>(g);
    m->entries["b"] = static_cast<double>(b);
    m->entries["a"] = static_cast<double>(a);
    return Value{m};
}

#ifdef HAVE_RAYLIB
// Extract a raylib Color from a color Map value
static ::Color extractRaylibColor(const Value& v) {
    if (!std::holds_alternative<std::shared_ptr<Map>>(v)) {
        throw std::runtime_error("Expected a color value from ui.color().");
    }
    auto m = std::get<std::shared_ptr<Map>>(v);
    auto getChannel = [&](const std::string& key, int defaultValue) -> unsigned char {
        auto it = m->entries.find(key);
        if (it != m->entries.end() && std::holds_alternative<double>(it->second)) {
            return static_cast<unsigned char>(std::get<double>(it->second));
        }
        return static_cast<unsigned char>(defaultValue);
    };
    return {getChannel("r", 0), getChannel("g", 0), getChannel("b", 0), getChannel("a", 255)};
}
#endif

// ---------------------------------------------------------------------------
// ui.color(r, g, b [, a])
// ---------------------------------------------------------------------------
static Value nativeUiColor(Interpreter& /*interp*/, const std::vector<Value>& args) {
    if (args.size() < 3 || args.size() > 4) {
        throw std::runtime_error("ui.color() takes 3 or 4 arguments (r, g, b [, a]).");
    }
    int r = static_cast<int>(asNumber(args[0]));
    int g = static_cast<int>(asNumber(args[1]));
    int b = static_cast<int>(asNumber(args[2]));
    int a = (args.size() == 4) ? static_cast<int>(asNumber(args[3])) : 255;
    return makeColorValue(r, g, b, a);
}

// ---------------------------------------------------------------------------
// ui.keyDown(key) / ui.keyPressed(key)
// ---------------------------------------------------------------------------
static Value nativeUiKeyDown(Interpreter& /*interp*/, const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("ui.keyDown() takes 1 argument.");
    }
#ifdef HAVE_RAYLIB
    int key = static_cast<int>(asNumber(args[0]));
    return static_cast<bool>(IsKeyDown(key));
#else
    (void)args;
    throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
}

static Value nativeUiKeyPressed(Interpreter& /*interp*/, const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("ui.keyPressed() takes 1 argument.");
    }
#ifdef HAVE_RAYLIB
    int key = static_cast<int>(asNumber(args[0]));
    return static_cast<bool>(IsKeyPressed(key));
#else
    (void)args;
    throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
}

// ---------------------------------------------------------------------------
// ui.mouseDown(button) / ui.mousePressed(button)
// ---------------------------------------------------------------------------
static Value nativeUiMouseDown(Interpreter& /*interp*/, const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("ui.mouseDown() takes 1 argument.");
    }
#ifdef HAVE_RAYLIB
    int btn = static_cast<int>(asNumber(args[0]));
    return static_cast<bool>(IsMouseButtonDown(btn));
#else
    (void)args;
    throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
}

static Value nativeUiMousePressed(Interpreter& /*interp*/, const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("ui.mousePressed() takes 1 argument.");
    }
#ifdef HAVE_RAYLIB
    int btn = static_cast<int>(asNumber(args[0]));
    return static_cast<bool>(IsMouseButtonPressed(btn));
#else
    (void)args;
    throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
}

// ---------------------------------------------------------------------------
// ui.getMousePosition() -> Map {x, y}
// ---------------------------------------------------------------------------
static Value nativeUiGetMousePosition(Interpreter& /*interp*/, const std::vector<Value>& args) {
    (void)args;
#ifdef HAVE_RAYLIB
    ::Vector2 pos = GetMousePosition();
    auto m = std::make_shared<Map>();
    m->entries["x"] = static_cast<double>(pos.x);
    m->entries["y"] = static_cast<double>(pos.y);
    return Value{m};
#else
    throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
}

// ---------------------------------------------------------------------------
// ui.getMouseWheelMove() -> float
// ---------------------------------------------------------------------------
static Value nativeUiGetMouseWheelMove(Interpreter& /*interp*/, const std::vector<Value>& args) {
    (void)args;
#ifdef HAVE_RAYLIB
    return static_cast<double>(GetMouseWheelMove());
#else
    throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
}

// ---------------------------------------------------------------------------
// ui.getCharPressed() -> int
// ---------------------------------------------------------------------------
static Value nativeUiGetCharPressed(Interpreter& /*interp*/, const std::vector<Value>& args) {
    (void)args;
#ifdef HAVE_RAYLIB
    return static_cast<double>(GetCharPressed());
#else
    throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
}

// ---------------------------------------------------------------------------
// Panel helper (virtual scissor region inside a single OS window)
// ---------------------------------------------------------------------------

static Value buildPanelObject(std::shared_ptr<UiPanel> panel) {
    auto obj = std::make_shared<Map>();

    // panel.begin() - activate scissor clipping for this panel
    obj->entries["begin"] = Value{std::make_shared<NativeFunction>("begin", 0,
        [panel](Interpreter&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            BeginScissorMode(panel->x, panel->y, panel->width, panel->height);
#endif
            return Nil{};
        })};

    // panel.end() - deactivate scissor clipping
    obj->entries["end"] = Value{std::make_shared<NativeFunction>("end", 0,
        [](Interpreter&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            EndScissorMode();
#endif
            return Nil{};
        })};

    // panel.getMousePosition() -> Map {x, y} in panel-local coordinates
    obj->entries["getMousePosition"] = Value{std::make_shared<NativeFunction>("getMousePosition", 0,
        [panel](Interpreter&, const std::vector<Value>&) -> Value {
            auto m = std::make_shared<Map>();
#ifdef HAVE_RAYLIB
            ::Vector2 pos = GetMousePosition();
            m->entries["x"] = static_cast<double>(pos.x - panel->x);
            m->entries["y"] = static_cast<double>(pos.y - panel->y);
#else
            m->entries["x"] = 0.0;
            m->entries["y"] = 0.0;
#endif
            return Value{m};
        })};

    // panel.containsMouse() -> bool
    obj->entries["containsMouse"] = Value{std::make_shared<NativeFunction>("containsMouse", 0,
        [panel](Interpreter&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            ::Vector2 pos = GetMousePosition();
            return static_cast<bool>(pos.x >= panel->x &&
                                     pos.x < panel->x + panel->width &&
                                     pos.y >= panel->y &&
                                     pos.y < panel->y + panel->height);
#else
            return false;
#endif
        })};

    // panel.drawText(x, y, text, fontSize, color) - panel-local coordinates
    obj->entries["drawText"] = Value{std::make_shared<NativeFunction>("drawText", 5,
        [panel](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 5) {
                throw std::runtime_error(
                    "panel.drawText() takes 5 arguments (x, y, text, fontSize, color).");
            }
#ifdef HAVE_RAYLIB
            int x = static_cast<int>(asNumber(args[0])) + panel->x;
            int y = static_cast<int>(asNumber(args[1])) + panel->y;
            std::string text = std::holds_alternative<std::string>(args[2])
                                   ? std::get<std::string>(args[2])
                                   : valueToString(args[2]);
            int fontSize = static_cast<int>(asNumber(args[3]));
            ::Color color = extractRaylibColor(args[4]);
            DrawText(text.c_str(), x, y, fontSize, color);
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    // panel.fillRect(x, y, width, height, color) - panel-local coordinates
    obj->entries["fillRect"] = Value{std::make_shared<NativeFunction>("fillRect", 5,
        [panel](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 5) {
                throw std::runtime_error(
                    "panel.fillRect() takes 5 arguments (x, y, width, height, color).");
            }
#ifdef HAVE_RAYLIB
            int x = static_cast<int>(asNumber(args[0])) + panel->x;
            int y = static_cast<int>(asNumber(args[1])) + panel->y;
            int w = static_cast<int>(asNumber(args[2]));
            int h = static_cast<int>(asNumber(args[3]));
            ::Color color = extractRaylibColor(args[4]);
            DrawRectangle(x, y, w, h, color);
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    // panel.drawRect(x, y, width, height, color) - panel-local coordinates
    obj->entries["drawRect"] = Value{std::make_shared<NativeFunction>("drawRect", 5,
        [panel](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 5) {
                throw std::runtime_error(
                    "panel.drawRect() takes 5 arguments (x, y, width, height, color).");
            }
#ifdef HAVE_RAYLIB
            int x = static_cast<int>(asNumber(args[0])) + panel->x;
            int y = static_cast<int>(asNumber(args[1])) + panel->y;
            int w = static_cast<int>(asNumber(args[2]));
            int h = static_cast<int>(asNumber(args[3]));
            ::Color color = extractRaylibColor(args[4]);
            DrawRectangleLines(x, y, w, h, color);
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    // panel.drawLine(x1, y1, x2, y2, thickness, color) - panel-local coordinates
    obj->entries["drawLine"] = Value{std::make_shared<NativeFunction>("drawLine", 6,
        [panel](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 6) {
                throw std::runtime_error(
                    "panel.drawLine() takes 6 arguments (x1, y1, x2, y2, thickness, color).");
            }
#ifdef HAVE_RAYLIB
            int x1 = static_cast<int>(asNumber(args[0])) + panel->x;
            int y1 = static_cast<int>(asNumber(args[1])) + panel->y;
            int x2 = static_cast<int>(asNumber(args[2])) + panel->x;
            int y2 = static_cast<int>(asNumber(args[3])) + panel->y;
            float thickness = static_cast<float>(asNumber(args[4]));
            ::Color color = extractRaylibColor(args[5]);
            DrawLineEx({static_cast<float>(x1), static_cast<float>(y1)},
                       {static_cast<float>(x2), static_cast<float>(y2)}, thickness, color);
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    // panel.drawCircle(x, y, radius, color) - panel-local coordinates
    obj->entries["drawCircle"] = Value{std::make_shared<NativeFunction>("drawCircle", 4,
        [panel](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 4) {
                throw std::runtime_error(
                    "panel.drawCircle() takes 4 arguments (x, y, radius, color).");
            }
#ifdef HAVE_RAYLIB
            int x = static_cast<int>(asNumber(args[0])) + panel->x;
            int y = static_cast<int>(asNumber(args[1])) + panel->y;
            float radius = static_cast<float>(asNumber(args[2]));
            ::Color color = extractRaylibColor(args[3]);
            DrawCircle(x, y, radius, color);
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    // panel.drawCircleLines(x, y, radius, color) - panel-local coordinates
    obj->entries["drawCircleLines"] = Value{std::make_shared<NativeFunction>("drawCircleLines", 4,
        [panel](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 4) {
                throw std::runtime_error(
                    "panel.drawCircleLines() takes 4 arguments (x, y, radius, color).");
            }
#ifdef HAVE_RAYLIB
            int x = static_cast<int>(asNumber(args[0])) + panel->x;
            int y = static_cast<int>(asNumber(args[1])) + panel->y;
            float radius = static_cast<float>(asNumber(args[2]));
            ::Color color = extractRaylibColor(args[3]);
            DrawCircleLines(x, y, radius, color);
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    // panel.fillRectRounded(x, y, width, height, roundness, color) - panel-local coordinates
    obj->entries["fillRectRounded"] = Value{std::make_shared<NativeFunction>("fillRectRounded", 6,
        [panel](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 6) {
                throw std::runtime_error(
                    "panel.fillRectRounded() takes 6 arguments (x, y, width, height, roundness, color).");
            }
#ifdef HAVE_RAYLIB
            float x = static_cast<float>(asNumber(args[0])) + static_cast<float>(panel->x);
            float y = static_cast<float>(asNumber(args[1])) + static_cast<float>(panel->y);
            float w = static_cast<float>(asNumber(args[2]));
            float h = static_cast<float>(asNumber(args[3]));
            float roundness = static_cast<float>(asNumber(args[4]));
            ::Color color = extractRaylibColor(args[5]);
            DrawRectangleRounded({x, y, w, h}, roundness, 8, color);
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    // panel.drawRectRounded(x, y, width, height, roundness, color) - panel-local coordinates
    obj->entries["drawRectRounded"] = Value{std::make_shared<NativeFunction>("drawRectRounded", 6,
        [panel](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 6) {
                throw std::runtime_error(
                    "panel.drawRectRounded() takes 6 arguments (x, y, width, height, roundness, color).");
            }
#ifdef HAVE_RAYLIB
            float x = static_cast<float>(asNumber(args[0])) + static_cast<float>(panel->x);
            float y = static_cast<float>(asNumber(args[1])) + static_cast<float>(panel->y);
            float w = static_cast<float>(asNumber(args[2]));
            float h = static_cast<float>(asNumber(args[3]));
            float roundness = static_cast<float>(asNumber(args[4]));
            ::Color color = extractRaylibColor(args[5]);
            DrawRectangleRoundedLines({x, y, w, h}, roundness, 8, color);
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    // panel.measureText(text, fontSize) -> number
    obj->entries["measureText"] = Value{std::make_shared<NativeFunction>("measureText", 2,
        [](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 2) {
                throw std::runtime_error(
                    "panel.measureText() takes 2 arguments (text, fontSize).");
            }
#ifdef HAVE_RAYLIB
            std::string text = std::holds_alternative<std::string>(args[0])
                                   ? std::get<std::string>(args[0])
                                   : valueToString(args[0]);
            int fontSize = static_cast<int>(asNumber(args[1]));
            return static_cast<double>(MeasureText(text.c_str(), fontSize));
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
        })};

    return Value{obj};
}

// ---------------------------------------------------------------------------
// Window method helpers (capture shared_ptr<UiWindow>)
// ---------------------------------------------------------------------------

static Value buildWindowObject(std::shared_ptr<UiWindow> win) {
    auto obj = std::make_shared<Map>();

    // win.isOpen() -> bool
    obj->entries["isOpen"] = Value{std::make_shared<NativeFunction>("isOpen", 0,
        [win](Interpreter&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            if (win->open) {
                win->open = !WindowShouldClose();
            }
#endif
            return static_cast<bool>(win->open);
        })};

    // win.close()
    obj->entries["close"] = Value{std::make_shared<NativeFunction>("close", 0,
        [win](Interpreter&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            if (win->open) {
                CloseWindow();
            }
#endif
            win->open = false;
            return Nil{};
        })};

    // win.beginDrawing()
    obj->entries["beginDrawing"] = Value{std::make_shared<NativeFunction>("beginDrawing", 0,
        [](Interpreter&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            BeginDrawing();
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    // win.endDrawing()
    obj->entries["endDrawing"] = Value{std::make_shared<NativeFunction>("endDrawing", 0,
        [](Interpreter&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            EndDrawing();
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    // win.clear(color)
    obj->entries["clear"] = Value{std::make_shared<NativeFunction>("clear", 1,
        [](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1) {
                throw std::runtime_error("win.clear() takes 1 argument.");
            }
#ifdef HAVE_RAYLIB
            ClearBackground(extractRaylibColor(args[0]));
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    // win.setTitle(title)
    obj->entries["setTitle"] = Value{std::make_shared<NativeFunction>("setTitle", 1,
        [win](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1 || !std::holds_alternative<std::string>(args[0])) {
                throw std::runtime_error("win.setTitle() takes 1 string argument.");
            }
            win->title = std::get<std::string>(args[0]);
#ifdef HAVE_RAYLIB
            SetWindowTitle(win->title.c_str());
#endif
            return Nil{};
        })};

    // win.getSize() -> Map {width, height}
    obj->entries["getSize"] = Value{std::make_shared<NativeFunction>("getSize", 0,
        [win](Interpreter&, const std::vector<Value>&) -> Value {
            auto m = std::make_shared<Map>();
#ifdef HAVE_RAYLIB
            win->width = GetScreenWidth();
            win->height = GetScreenHeight();
#endif
            m->entries["width"] = static_cast<double>(win->width);
            m->entries["height"] = static_cast<double>(win->height);
            return Value{m};
        })};

    // win.getFps() -> int
    obj->entries["getFps"] = Value{std::make_shared<NativeFunction>("getFps", 0,
        [](Interpreter&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            return static_cast<double>(GetFPS());
#else
            return 0.0;
#endif
        })};

    // win.setTargetFPS(fps) - set the target frames-per-second for the game loop
    obj->entries["setTargetFPS"] = Value{std::make_shared<NativeFunction>("setTargetFPS", 1,
        [](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1) {
                throw std::runtime_error("win.setTargetFPS() takes 1 argument (fps).");
            }
#ifdef HAVE_RAYLIB
            SetTargetFPS(static_cast<int>(asNumber(args[0])));
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    // win.getFrameTime() -> float - get time in seconds for last frame drawn (delta time)
    obj->entries["getFrameTime"] = Value{std::make_shared<NativeFunction>("getFrameTime", 0,
        [](Interpreter&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            return static_cast<double>(GetFrameTime());
#else
            return 0.0;
#endif
        })};

    // win.drawText(x, y, text, fontSize, color)
    obj->entries["drawText"] = Value{std::make_shared<NativeFunction>("drawText", 5,
        [](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 5) {
                throw std::runtime_error("win.drawText() takes 5 arguments (x, y, text, fontSize, color).");
            }
#ifdef HAVE_RAYLIB
            int x = static_cast<int>(asNumber(args[0]));
            int y = static_cast<int>(asNumber(args[1]));
            std::string text = std::holds_alternative<std::string>(args[2])
                                   ? std::get<std::string>(args[2])
                                   : valueToString(args[2]);
            int fontSize = static_cast<int>(asNumber(args[3]));
            ::Color color = extractRaylibColor(args[4]);
            DrawText(text.c_str(), x, y, fontSize, color);
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    // win.drawRect(x, y, width, height, color)
    obj->entries["drawRect"] = Value{std::make_shared<NativeFunction>("drawRect", 5,
        [](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 5) {
                throw std::runtime_error("win.drawRect() takes 5 arguments (x, y, width, height, color).");
            }
#ifdef HAVE_RAYLIB
            int x = static_cast<int>(asNumber(args[0]));
            int y = static_cast<int>(asNumber(args[1]));
            int w = static_cast<int>(asNumber(args[2]));
            int h = static_cast<int>(asNumber(args[3]));
            ::Color color = extractRaylibColor(args[4]);
            DrawRectangleLines(x, y, w, h, color);
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    // win.fillRect(x, y, width, height, color)
    obj->entries["fillRect"] = Value{std::make_shared<NativeFunction>("fillRect", 5,
        [](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 5) {
                throw std::runtime_error("win.fillRect() takes 5 arguments (x, y, width, height, color).");
            }
#ifdef HAVE_RAYLIB
            int x = static_cast<int>(asNumber(args[0]));
            int y = static_cast<int>(asNumber(args[1]));
            int w = static_cast<int>(asNumber(args[2]));
            int h = static_cast<int>(asNumber(args[3]));
            ::Color color = extractRaylibColor(args[4]);
            DrawRectangle(x, y, w, h, color);
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    // win.drawLine(x1, y1, x2, y2, thickness, color)
    obj->entries["drawLine"] = Value{std::make_shared<NativeFunction>("drawLine", 6,
        [](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 6) {
                throw std::runtime_error("win.drawLine() takes 6 arguments (x1, y1, x2, y2, thickness, color).");
            }
#ifdef HAVE_RAYLIB
            int x1 = static_cast<int>(asNumber(args[0]));
            int y1 = static_cast<int>(asNumber(args[1]));
            int x2 = static_cast<int>(asNumber(args[2]));
            int y2 = static_cast<int>(asNumber(args[3]));
            float thickness = static_cast<float>(asNumber(args[4]));
            ::Color color = extractRaylibColor(args[5]);
            DrawLineEx({static_cast<float>(x1), static_cast<float>(y1)},
                       {static_cast<float>(x2), static_cast<float>(y2)}, thickness, color);
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    // win.drawCircle(x, y, radius, color)
    obj->entries["drawCircle"] = Value{std::make_shared<NativeFunction>("drawCircle", 4,
        [](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 4) {
                throw std::runtime_error("win.drawCircle() takes 4 arguments (x, y, radius, color).");
            }
#ifdef HAVE_RAYLIB
            int x = static_cast<int>(asNumber(args[0]));
            int y = static_cast<int>(asNumber(args[1]));
            float radius = static_cast<float>(asNumber(args[2]));
            ::Color color = extractRaylibColor(args[3]);
            DrawCircle(x, y, radius, color);
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    // win.drawCircleLines(x, y, radius, color)
    obj->entries["drawCircleLines"] = Value{std::make_shared<NativeFunction>("drawCircleLines", 4,
        [](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 4) {
                throw std::runtime_error("win.drawCircleLines() takes 4 arguments (x, y, radius, color).");
            }
#ifdef HAVE_RAYLIB
            int x = static_cast<int>(asNumber(args[0]));
            int y = static_cast<int>(asNumber(args[1]));
            float radius = static_cast<float>(asNumber(args[2]));
            ::Color color = extractRaylibColor(args[3]);
            DrawCircleLines(x, y, radius, color);
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    // win.fillRectRounded(x, y, width, height, roundness, color)
    obj->entries["fillRectRounded"] = Value{std::make_shared<NativeFunction>("fillRectRounded", 6,
        [](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 6) {
                throw std::runtime_error("win.fillRectRounded() takes 6 arguments (x, y, width, height, roundness, color).");
            }
#ifdef HAVE_RAYLIB
            float x = static_cast<float>(asNumber(args[0]));
            float y = static_cast<float>(asNumber(args[1]));
            float w = static_cast<float>(asNumber(args[2]));
            float h = static_cast<float>(asNumber(args[3]));
            float roundness = static_cast<float>(asNumber(args[4]));
            ::Color color = extractRaylibColor(args[5]);
            DrawRectangleRounded({x, y, w, h}, roundness, 8, color);
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    // win.drawRectRounded(x, y, width, height, roundness, color)
    obj->entries["drawRectRounded"] = Value{std::make_shared<NativeFunction>("drawRectRounded", 6,
        [](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 6) {
                throw std::runtime_error("win.drawRectRounded() takes 6 arguments (x, y, width, height, roundness, color).");
            }
#ifdef HAVE_RAYLIB
            float x = static_cast<float>(asNumber(args[0]));
            float y = static_cast<float>(asNumber(args[1]));
            float w = static_cast<float>(asNumber(args[2]));
            float h = static_cast<float>(asNumber(args[3]));
            float roundness = static_cast<float>(asNumber(args[4]));
            ::Color color = extractRaylibColor(args[5]);
            DrawRectangleRoundedLines({x, y, w, h}, roundness, 8, color);
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    // win.measureText(text, fontSize) -> number
    obj->entries["measureText"] = Value{std::make_shared<NativeFunction>("measureText", 2,
        [](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 2) {
                throw std::runtime_error("win.measureText() takes 2 arguments (text, fontSize).");
            }
#ifdef HAVE_RAYLIB
            std::string text = std::holds_alternative<std::string>(args[0])
                                   ? std::get<std::string>(args[0])
                                   : valueToString(args[0]);
            int fontSize = static_cast<int>(asNumber(args[1]));
            return static_cast<double>(MeasureText(text.c_str(), fontSize));
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
        })};

    // win.toggleFullscreen()
    obj->entries["toggleFullscreen"] = Value{std::make_shared<NativeFunction>("toggleFullscreen", 0,
        [](Interpreter&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            ToggleFullscreen();
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    // win.isWindowFocused() -> bool
    obj->entries["isWindowFocused"] = Value{std::make_shared<NativeFunction>("isWindowFocused", 0,
        [](Interpreter&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            return static_cast<bool>(IsWindowFocused());
#else
            return false;
#endif
        })};

    // win.setWindowMinSize(width, height)
    obj->entries["setWindowMinSize"] = Value{std::make_shared<NativeFunction>("setWindowMinSize", 2,
        [](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 2) {
                throw std::runtime_error("win.setWindowMinSize() takes 2 arguments (width, height).");
            }
#ifdef HAVE_RAYLIB
            SetWindowMinSize(static_cast<int>(asNumber(args[0])), static_cast<int>(asNumber(args[1])));
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    // win.createPanel(x, y, width, height) -> Panel
    obj->entries["createPanel"] = Value{std::make_shared<NativeFunction>("createPanel", 4,
        [](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 4) {
                throw std::runtime_error(
                    "win.createPanel() takes 4 arguments (x, y, width, height).");
            }
            auto p = std::make_shared<UiPanel>();
            p->x = static_cast<int>(asNumber(args[0]));
            p->y = static_cast<int>(asNumber(args[1]));
            p->width = static_cast<int>(asNumber(args[2]));
            p->height = static_cast<int>(asNumber(args[3]));
            return buildPanelObject(p);
        })};

    return Value{obj};
}

// ---------------------------------------------------------------------------
// Camera2D state
// ---------------------------------------------------------------------------
struct UiCamera2D {
    float offsetX = 0.0f, offsetY = 0.0f;
    float targetX = 0.0f, targetY = 0.0f;
    float rotation = 0.0f;
    float zoom = 1.0f;
};

static Value buildCamera2DObject(std::shared_ptr<UiCamera2D> cam) {
    auto obj = std::make_shared<Map>();

    obj->entries["setTarget"] = Value{std::make_shared<NativeFunction>("setTarget", 2,
        [cam](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 2) {
                throw std::runtime_error("camera.setTarget() takes 2 arguments (x, y).");
            }
            cam->targetX = static_cast<float>(asNumber(args[0]));
            cam->targetY = static_cast<float>(asNumber(args[1]));
            return Nil{};
        })};

    obj->entries["setOffset"] = Value{std::make_shared<NativeFunction>("setOffset", 2,
        [cam](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 2) {
                throw std::runtime_error("camera.setOffset() takes 2 arguments (x, y).");
            }
            cam->offsetX = static_cast<float>(asNumber(args[0]));
            cam->offsetY = static_cast<float>(asNumber(args[1]));
            return Nil{};
        })};

    obj->entries["setRotation"] = Value{std::make_shared<NativeFunction>("setRotation", 1,
        [cam](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1) {
                throw std::runtime_error("camera.setRotation() takes 1 argument (degrees).");
            }
            // Rotation is in degrees, matching raylib's Camera2D convention
            cam->rotation = static_cast<float>(asNumber(args[0]));
            return Nil{};
        })};

    obj->entries["setZoom"] = Value{std::make_shared<NativeFunction>("setZoom", 1,
        [cam](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1) {
                throw std::runtime_error("camera.setZoom() takes 1 argument (zoom).");
            }
            cam->zoom = static_cast<float>(asNumber(args[0]));
            return Nil{};
        })};

    obj->entries["beginMode"] = Value{std::make_shared<NativeFunction>("beginMode", 0,
        [cam](Interpreter&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            ::Camera2D c{};
            c.offset = {cam->offsetX, cam->offsetY};
            c.target = {cam->targetX, cam->targetY};
            c.rotation = cam->rotation;
            c.zoom = cam->zoom;
            BeginMode2D(c);
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    obj->entries["endMode"] = Value{std::make_shared<NativeFunction>("endMode", 0,
        [](Interpreter&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            EndMode2D();
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    obj->entries["getWorldToScreen"] = Value{std::make_shared<NativeFunction>("getWorldToScreen", 2,
        [cam](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 2) {
                throw std::runtime_error("camera.getWorldToScreen() takes 2 arguments (x, y).");
            }
            auto m = std::make_shared<Map>();
#ifdef HAVE_RAYLIB
            ::Camera2D c{};
            c.offset = {cam->offsetX, cam->offsetY};
            c.target = {cam->targetX, cam->targetY};
            c.rotation = cam->rotation;
            c.zoom = cam->zoom;
            ::Vector2 world{static_cast<float>(asNumber(args[0])),
                            static_cast<float>(asNumber(args[1]))};
            ::Vector2 screen = GetWorldToScreen2D(world, c);
            m->entries["x"] = static_cast<double>(screen.x);
            m->entries["y"] = static_cast<double>(screen.y);
#else
            m->entries["x"] = asNumber(args[0]);
            m->entries["y"] = asNumber(args[1]);
#endif
            return Value{m};
        })};

    obj->entries["getScreenToWorld"] = Value{std::make_shared<NativeFunction>("getScreenToWorld", 2,
        [cam](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 2) {
                throw std::runtime_error("camera.getScreenToWorld() takes 2 arguments (x, y).");
            }
            auto m = std::make_shared<Map>();
#ifdef HAVE_RAYLIB
            ::Camera2D c{};
            c.offset = {cam->offsetX, cam->offsetY};
            c.target = {cam->targetX, cam->targetY};
            c.rotation = cam->rotation;
            c.zoom = cam->zoom;
            ::Vector2 screen{static_cast<float>(asNumber(args[0])),
                             static_cast<float>(asNumber(args[1]))};
            ::Vector2 world = GetScreenToWorld2D(screen, c);
            m->entries["x"] = static_cast<double>(world.x);
            m->entries["y"] = static_cast<double>(world.y);
#else
            m->entries["x"] = asNumber(args[0]);
            m->entries["y"] = asNumber(args[1]);
#endif
            return Value{m};
        })};

    return Value{obj};
}

// ---------------------------------------------------------------------------
// Texture wrapper
// ---------------------------------------------------------------------------
#ifdef HAVE_RAYLIB
struct UiTexture {
    ::Texture2D texture{};
    bool loaded = false;
    ~UiTexture() {
        if (loaded) {
            UnloadTexture(texture);
            loaded = false;
        }
    }
};

static Value buildTextureObject(std::shared_ptr<UiTexture> tex) {
    auto obj = std::make_shared<Map>();

    obj->entries["getWidth"] = Value{std::make_shared<NativeFunction>("getWidth", 0,
        [tex](Interpreter&, const std::vector<Value>&) -> Value {
            return tex->loaded ? static_cast<double>(tex->texture.width) : 0.0;
        })};

    obj->entries["getHeight"] = Value{std::make_shared<NativeFunction>("getHeight", 0,
        [tex](Interpreter&, const std::vector<Value>&) -> Value {
            return tex->loaded ? static_cast<double>(tex->texture.height) : 0.0;
        })};

    obj->entries["draw"] = Value{std::make_shared<NativeFunction>("draw", 3,
        [tex](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 3) {
                throw std::runtime_error("texture.draw() takes 3 arguments (x, y, tint).");
            }
            if (tex->loaded) {
                int x = static_cast<int>(asNumber(args[0]));
                int y = static_cast<int>(asNumber(args[1]));
                ::Color tint = extractRaylibColor(args[2]);
                DrawTexture(tex->texture, x, y, tint);
            }
            return Nil{};
        })};

    obj->entries["drawEx"] = Value{std::make_shared<NativeFunction>("drawEx", 5,
        [tex](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 5) {
                throw std::runtime_error(
                    "texture.drawEx() takes 5 arguments (x, y, rotation, scale, tint).");
            }
            if (tex->loaded) {
                float x = static_cast<float>(asNumber(args[0]));
                float y = static_cast<float>(asNumber(args[1]));
                float rotation = static_cast<float>(asNumber(args[2]));
                float scale = static_cast<float>(asNumber(args[3]));
                ::Color tint = extractRaylibColor(args[4]);
                DrawTextureEx(tex->texture, {x, y}, rotation, scale, tint);
            }
            return Nil{};
        })};

    obj->entries["drawRec"] = Value{std::make_shared<NativeFunction>("drawRec", 4,
        [tex](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 4) {
                throw std::runtime_error(
                    "texture.drawRec() takes 4 arguments (srcRec, position, rotation, tint). "
                    "srcRec and position are {x, y, width, height} and {x, y} maps respectively.");
            }
            if (tex->loaded) {
                if (!std::holds_alternative<std::shared_ptr<Map>>(args[0]) ||
                    !std::holds_alternative<std::shared_ptr<Map>>(args[1])) {
                    throw std::runtime_error("texture.drawRec(): srcRec and position must be maps.");
                }
                auto srcMap = std::get<std::shared_ptr<Map>>(args[0]);
                auto posMap = std::get<std::shared_ptr<Map>>(args[1]);
                auto getNum = [](const std::shared_ptr<Map>& m, const std::string& k) -> float {
                    auto it = m->entries.find(k);
                    return (it != m->entries.end() && std::holds_alternative<double>(it->second))
                               ? static_cast<float>(std::get<double>(it->second))
                               : 0.0f;
                };
                ::Rectangle src{getNum(srcMap, "x"), getNum(srcMap, "y"),
                                getNum(srcMap, "width"), getNum(srcMap, "height")};
                ::Vector2 pos{getNum(posMap, "x"), getNum(posMap, "y")};
                float rotation = static_cast<float>(asNumber(args[2]));
                ::Color tint = extractRaylibColor(args[3]);
                DrawTextureRec(tex->texture, src, pos, tint);
            }
            return Nil{};
        })};

    obj->entries["unload"] = Value{std::make_shared<NativeFunction>("unload", 0,
        [tex](Interpreter&, const std::vector<Value>&) -> Value {
            if (tex->loaded) {
                UnloadTexture(tex->texture);
                tex->loaded = false;
            }
            return Nil{};
        })};

    return Value{obj};
}
#endif

// ---------------------------------------------------------------------------
// ui.createCamera2D(offsetX, offsetY, targetX, targetY, rotation, zoom)
// ---------------------------------------------------------------------------
static Value nativeUiCreateCamera2D(Interpreter& /*interp*/, const std::vector<Value>& args) {
    if (args.size() != 6) {
        throw std::runtime_error(
            "ui.createCamera2D() takes 6 arguments (offsetX, offsetY, targetX, targetY, rotation, zoom).");
    }
    auto cam = std::make_shared<UiCamera2D>();
    cam->offsetX   = static_cast<float>(asNumber(args[0]));
    cam->offsetY   = static_cast<float>(asNumber(args[1]));
    cam->targetX   = static_cast<float>(asNumber(args[2]));
    cam->targetY   = static_cast<float>(asNumber(args[3]));
    cam->rotation  = static_cast<float>(asNumber(args[4]));
    cam->zoom      = static_cast<float>(asNumber(args[5]));
    return buildCamera2DObject(cam);
}

// ---------------------------------------------------------------------------
// ui.loadTexture(path) -> texture object
// ---------------------------------------------------------------------------
static Value nativeUiLoadTexture(Interpreter& /*interp*/, const std::vector<Value>& args) {
    if (args.size() != 1 || !std::holds_alternative<std::string>(args[0])) {
        throw std::runtime_error("ui.loadTexture() takes 1 string argument (path).");
    }
#ifdef HAVE_RAYLIB
    const std::string& path = std::get<std::string>(args[0]);
    auto tex = std::make_shared<UiTexture>();
    tex->texture = LoadTexture(path.c_str());
    tex->loaded = true;
    return buildTextureObject(tex);
#else
    throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
}

// ---------------------------------------------------------------------------
// ui.createWindow(title, width, height)
// ---------------------------------------------------------------------------
static Value nativeUiCreateWindow(Interpreter& /*interp*/, const std::vector<Value>& args) {
    if (args.size() != 3) {
        throw std::runtime_error("ui.createWindow() takes 3 arguments (title, width, height).");
    }
    if (!std::holds_alternative<std::string>(args[0])) {
        throw std::runtime_error("ui.createWindow(): title must be a string.");
    }
    std::string title = std::get<std::string>(args[0]);
    int width = static_cast<int>(asNumber(args[1]));
    int height = static_cast<int>(asNumber(args[2]));
    auto win = std::make_shared<UiWindow>();
    win->width = width;
    win->height = height;
    win->title = title;
#ifdef HAVE_RAYLIB
    if (IsWindowReady()) {
        throw std::runtime_error("ui.createWindow(): only one window is supported at a time.");
    }
    InitWindow(width, height, title.c_str());
    if (!IsWindowReady()) {
        throw std::runtime_error(
            "ui.createWindow(): failed to initialize window (no graphical display available?).");
    }
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);
    win->open = true;
#endif
    return buildWindowObject(win);
}

// ---------------------------------------------------------------------------
// Build the top-level ui module Map
// ---------------------------------------------------------------------------
Value createUiModule(Interpreter& interp) {
    auto module = std::make_shared<Map>();

    // Window creation
    module->entries["createWindow"] =
        Value{std::make_shared<NativeFunction>("createWindow", 3, nativeUiCreateWindow)};

    // Color
    module->entries["color"] = Value{std::make_shared<NativeFunction>("color", -1, nativeUiColor)};

    // Keyboard input
    module->entries["keyDown"] = Value{std::make_shared<NativeFunction>("keyDown", 1, nativeUiKeyDown)};
    module->entries["keyPressed"] = Value{std::make_shared<NativeFunction>("keyPressed", 1, nativeUiKeyPressed)};

    // Mouse input
    module->entries["mouseDown"] = Value{std::make_shared<NativeFunction>("mouseDown", 1, nativeUiMouseDown)};
    module->entries["mousePressed"] = Value{std::make_shared<NativeFunction>("mousePressed", 1, nativeUiMousePressed)};
    module->entries["getMousePosition"] =
        Value{std::make_shared<NativeFunction>("getMousePosition", 0, nativeUiGetMousePosition)};
    module->entries["getMouseWheelMove"] =
        Value{std::make_shared<NativeFunction>("getMouseWheelMove", 0, nativeUiGetMouseWheelMove)};
    module->entries["getCharPressed"] =
        Value{std::make_shared<NativeFunction>("getCharPressed", 0, nativeUiGetCharPressed)};

    // Camera 2D
    module->entries["createCamera2D"] =
        Value{std::make_shared<NativeFunction>("createCamera2D", 6, nativeUiCreateCamera2D)};

    // Texture loading
    module->entries["loadTexture"] =
        Value{std::make_shared<NativeFunction>("loadTexture", 1, nativeUiLoadTexture)};

    // ui.getTime() -> number  (seconds since window was initialized)
    module->entries["getTime"] = Value{std::make_shared<NativeFunction>("getTime", 0,
        [](Interpreter&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            return static_cast<double>(GetTime());
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
        })};

    // Key constants sub-map
    {
        auto keys = std::make_shared<Map>();
#ifdef HAVE_RAYLIB
        keys->entries["escape"] = static_cast<double>(KEY_ESCAPE);
        keys->entries["enter"] = static_cast<double>(KEY_ENTER);
        keys->entries["space"] = static_cast<double>(KEY_SPACE);
        keys->entries["backspace"] = static_cast<double>(KEY_BACKSPACE);
        keys->entries["tab"] = static_cast<double>(KEY_TAB);
        keys->entries["delete"] = static_cast<double>(KEY_DELETE);
        keys->entries["left"] = static_cast<double>(KEY_LEFT);
        keys->entries["right"] = static_cast<double>(KEY_RIGHT);
        keys->entries["up"] = static_cast<double>(KEY_UP);
        keys->entries["down"] = static_cast<double>(KEY_DOWN);
        keys->entries["leftShift"] = static_cast<double>(KEY_LEFT_SHIFT);
        keys->entries["leftCtrl"] = static_cast<double>(KEY_LEFT_CONTROL);
        keys->entries["leftAlt"] = static_cast<double>(KEY_LEFT_ALT);
        keys->entries["rightShift"] = static_cast<double>(KEY_RIGHT_SHIFT);
        keys->entries["rightCtrl"] = static_cast<double>(KEY_RIGHT_CONTROL);
        keys->entries["rightAlt"] = static_cast<double>(KEY_RIGHT_ALT);
        keys->entries["a"] = static_cast<double>(KEY_A);
        keys->entries["b"] = static_cast<double>(KEY_B);
        keys->entries["c"] = static_cast<double>(KEY_C);
        keys->entries["d"] = static_cast<double>(KEY_D);
        keys->entries["e"] = static_cast<double>(KEY_E);
        keys->entries["f"] = static_cast<double>(KEY_F);
        keys->entries["g"] = static_cast<double>(KEY_G);
        keys->entries["h"] = static_cast<double>(KEY_H);
        keys->entries["i"] = static_cast<double>(KEY_I);
        keys->entries["j"] = static_cast<double>(KEY_J);
        keys->entries["k"] = static_cast<double>(KEY_K);
        keys->entries["l"] = static_cast<double>(KEY_L);
        keys->entries["m"] = static_cast<double>(KEY_M);
        keys->entries["n"] = static_cast<double>(KEY_N);
        keys->entries["o"] = static_cast<double>(KEY_O);
        keys->entries["p"] = static_cast<double>(KEY_P);
        keys->entries["q"] = static_cast<double>(KEY_Q);
        keys->entries["r"] = static_cast<double>(KEY_R);
        keys->entries["s"] = static_cast<double>(KEY_S);
        keys->entries["t"] = static_cast<double>(KEY_T);
        keys->entries["u"] = static_cast<double>(KEY_U);
        keys->entries["v"] = static_cast<double>(KEY_V);
        keys->entries["w"] = static_cast<double>(KEY_W);
        keys->entries["x"] = static_cast<double>(KEY_X);
        keys->entries["y"] = static_cast<double>(KEY_Y);
        keys->entries["z"] = static_cast<double>(KEY_Z);
        keys->entries["f1"] = static_cast<double>(KEY_F1);
        keys->entries["f2"] = static_cast<double>(KEY_F2);
        keys->entries["f3"] = static_cast<double>(KEY_F3);
        keys->entries["f4"] = static_cast<double>(KEY_F4);
        keys->entries["f5"] = static_cast<double>(KEY_F5);
        keys->entries["f6"] = static_cast<double>(KEY_F6);
        keys->entries["f7"] = static_cast<double>(KEY_F7);
        keys->entries["f8"] = static_cast<double>(KEY_F8);
        keys->entries["f9"] = static_cast<double>(KEY_F9);
        keys->entries["f10"] = static_cast<double>(KEY_F10);
        keys->entries["f11"] = static_cast<double>(KEY_F11);
        keys->entries["f12"] = static_cast<double>(KEY_F12);
#else
        // Provide standard key code values (raylib compatible) even without raylib
        keys->entries["escape"] = 256.0;
        keys->entries["enter"] = 257.0;
        keys->entries["space"] = 32.0;
        keys->entries["backspace"] = 259.0;
        keys->entries["tab"] = 258.0;
        keys->entries["delete"] = 261.0;
        keys->entries["left"] = 263.0;
        keys->entries["right"] = 262.0;
        keys->entries["up"] = 265.0;
        keys->entries["down"] = 264.0;
        keys->entries["leftShift"] = 340.0;
        keys->entries["leftCtrl"] = 341.0;
        keys->entries["leftAlt"] = 342.0;
        keys->entries["rightShift"] = 344.0;
        keys->entries["rightCtrl"] = 345.0;
        keys->entries["rightAlt"] = 346.0;
        keys->entries["a"] = 65.0;
        keys->entries["b"] = 66.0;
        keys->entries["c"] = 67.0;
        keys->entries["d"] = 68.0;
        keys->entries["e"] = 69.0;
        keys->entries["f"] = 70.0;
        keys->entries["g"] = 71.0;
        keys->entries["h"] = 72.0;
        keys->entries["i"] = 73.0;
        keys->entries["j"] = 74.0;
        keys->entries["k"] = 75.0;
        keys->entries["l"] = 76.0;
        keys->entries["m"] = 77.0;
        keys->entries["n"] = 78.0;
        keys->entries["o"] = 79.0;
        keys->entries["p"] = 80.0;
        keys->entries["q"] = 81.0;
        keys->entries["r"] = 82.0;
        keys->entries["s"] = 83.0;
        keys->entries["t"] = 84.0;
        keys->entries["u"] = 85.0;
        keys->entries["v"] = 86.0;
        keys->entries["w"] = 87.0;
        keys->entries["x"] = 88.0;
        keys->entries["y"] = 89.0;
        keys->entries["z"] = 90.0;
        keys->entries["f1"] = 290.0;
        keys->entries["f2"] = 291.0;
        keys->entries["f3"] = 292.0;
        keys->entries["f4"] = 293.0;
        keys->entries["f5"] = 294.0;
        keys->entries["f6"] = 295.0;
        keys->entries["f7"] = 296.0;
        keys->entries["f8"] = 297.0;
        keys->entries["f9"] = 298.0;
        keys->entries["f10"] = 299.0;
        keys->entries["f11"] = 300.0;
        keys->entries["f12"] = 301.0;
#endif
        module->entries["key"] = Value{keys};
    }

    // Mouse button constants sub-map
    {
        auto mouse = std::make_shared<Map>();
#ifdef HAVE_RAYLIB
        mouse->entries["left"] = static_cast<double>(MOUSE_BUTTON_LEFT);
        mouse->entries["right"] = static_cast<double>(MOUSE_BUTTON_RIGHT);
        mouse->entries["middle"] = static_cast<double>(MOUSE_BUTTON_MIDDLE);
#else
        mouse->entries["left"] = 0.0;
        mouse->entries["right"] = 1.0;
        mouse->entries["middle"] = 2.0;
#endif
        module->entries["mouse"] = Value{mouse};
    }

    return Value{module};
}

}  // namespace izi
