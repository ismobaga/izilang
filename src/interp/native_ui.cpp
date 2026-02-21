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
                win->open = false;
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

    return Value{obj};
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
#ifdef HAVE_RAYLIB
    if (IsWindowReady()) {
        throw std::runtime_error("ui.createWindow(): only one window is supported at a time.");
    }
    std::string title = std::get<std::string>(args[0]);
    int width = static_cast<int>(asNumber(args[1]));
    int height = static_cast<int>(asNumber(args[2]));
    InitWindow(width, height, title.c_str());
    SetTargetFPS(60);
    auto win = std::make_shared<UiWindow>();
    win->open = true;
    win->width = width;
    win->height = height;
    win->title = title;
    return buildWindowObject(win);
#else
    throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
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

    // Key constants sub-map
    {
        auto keys = std::make_shared<Map>();
#ifdef HAVE_RAYLIB
        keys->entries["escape"] = static_cast<double>(KEY_ESCAPE);
        keys->entries["enter"] = static_cast<double>(KEY_ENTER);
        keys->entries["space"] = static_cast<double>(KEY_SPACE);
        keys->entries["left"] = static_cast<double>(KEY_LEFT);
        keys->entries["right"] = static_cast<double>(KEY_RIGHT);
        keys->entries["up"] = static_cast<double>(KEY_UP);
        keys->entries["down"] = static_cast<double>(KEY_DOWN);
        keys->entries["a"] = static_cast<double>(KEY_A);
        keys->entries["b"] = static_cast<double>(KEY_B);
        keys->entries["w"] = static_cast<double>(KEY_W);
        keys->entries["s"] = static_cast<double>(KEY_S);
        keys->entries["d"] = static_cast<double>(KEY_D);
#else
        // Provide standard key code values (raylib compatible) even without raylib
        keys->entries["escape"] = 256.0;
        keys->entries["enter"] = 257.0;
        keys->entries["space"] = 32.0;
        keys->entries["left"] = 263.0;
        keys->entries["right"] = 262.0;
        keys->entries["up"] = 265.0;
        keys->entries["down"] = 264.0;
        keys->entries["a"] = 65.0;
        keys->entries["b"] = 66.0;
        keys->entries["w"] = 87.0;
        keys->entries["s"] = 83.0;
        keys->entries["d"] = 68.0;
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
