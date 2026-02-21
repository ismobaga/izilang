#include "vm_native_ui.hpp"
#include "vm_native.hpp"
#include "vm.hpp"
#include <memory>
#include <string>

#ifdef HAVE_RAYLIB
#include <raylib.h>
#endif

namespace izi {

// Internal window state
struct VmUiWindow {
    bool open = false;
    int width = 0;
    int height = 0;
    std::string title;
};

// Internal panel state (virtual window / scissor region inside a VmUiWindow)
struct VmUiPanel {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
};

// ---------------------------------------------------------------------------
// Color helpers
// ---------------------------------------------------------------------------

static Value vmMakeColorValue(int r, int g, int b, int a = 255) {
    auto m = std::make_shared<Map>();
    m->entries["r"] = static_cast<double>(r);
    m->entries["g"] = static_cast<double>(g);
    m->entries["b"] = static_cast<double>(b);
    m->entries["a"] = static_cast<double>(a);
    return Value{m};
}

#ifdef HAVE_RAYLIB
static ::Color vmExtractRaylibColor(const Value& v) {
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
// Panel helper
// ---------------------------------------------------------------------------

static Value vmBuildPanelObject(std::shared_ptr<VmUiPanel> panel) {
    auto obj = std::make_shared<Map>();

    obj->entries["begin"] = Value{std::make_shared<VmNativeFunction>("begin", 0,
        [panel](VM&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            BeginScissorMode(panel->x, panel->y, panel->width, panel->height);
#endif
            return Nil{};
        })};

    obj->entries["end"] = Value{std::make_shared<VmNativeFunction>("end", 0,
        [](VM&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            EndScissorMode();
#endif
            return Nil{};
        })};

    obj->entries["getMousePosition"] = Value{std::make_shared<VmNativeFunction>("getMousePosition", 0,
        [panel](VM&, const std::vector<Value>&) -> Value {
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

    obj->entries["containsMouse"] = Value{std::make_shared<VmNativeFunction>("containsMouse", 0,
        [panel](VM&, const std::vector<Value>&) -> Value {
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

    obj->entries["drawText"] = Value{std::make_shared<VmNativeFunction>("drawText", 5,
        [panel](VM&, const std::vector<Value>& args) -> Value {
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
            ::Color color = vmExtractRaylibColor(args[4]);
            DrawText(text.c_str(), x, y, fontSize, color);
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    obj->entries["fillRect"] = Value{std::make_shared<VmNativeFunction>("fillRect", 5,
        [panel](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 5) {
                throw std::runtime_error(
                    "panel.fillRect() takes 5 arguments (x, y, width, height, color).");
            }
#ifdef HAVE_RAYLIB
            int x = static_cast<int>(asNumber(args[0])) + panel->x;
            int y = static_cast<int>(asNumber(args[1])) + panel->y;
            int w = static_cast<int>(asNumber(args[2]));
            int h = static_cast<int>(asNumber(args[3]));
            ::Color color = vmExtractRaylibColor(args[4]);
            DrawRectangle(x, y, w, h, color);
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    obj->entries["drawRect"] = Value{std::make_shared<VmNativeFunction>("drawRect", 5,
        [panel](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 5) {
                throw std::runtime_error(
                    "panel.drawRect() takes 5 arguments (x, y, width, height, color).");
            }
#ifdef HAVE_RAYLIB
            int x = static_cast<int>(asNumber(args[0])) + panel->x;
            int y = static_cast<int>(asNumber(args[1])) + panel->y;
            int w = static_cast<int>(asNumber(args[2]));
            int h = static_cast<int>(asNumber(args[3]));
            ::Color color = vmExtractRaylibColor(args[4]);
            DrawRectangleLines(x, y, w, h, color);
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    obj->entries["drawLine"] = Value{std::make_shared<VmNativeFunction>("drawLine", 6,
        [panel](VM&, const std::vector<Value>& args) -> Value {
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
            ::Color color = vmExtractRaylibColor(args[5]);
            DrawLineEx({static_cast<float>(x1), static_cast<float>(y1)},
                       {static_cast<float>(x2), static_cast<float>(y2)}, thickness, color);
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    obj->entries["drawCircle"] = Value{std::make_shared<VmNativeFunction>("drawCircle", 4,
        [panel](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 4) {
                throw std::runtime_error(
                    "panel.drawCircle() takes 4 arguments (x, y, radius, color).");
            }
#ifdef HAVE_RAYLIB
            int x = static_cast<int>(asNumber(args[0])) + panel->x;
            int y = static_cast<int>(asNumber(args[1])) + panel->y;
            float radius = static_cast<float>(asNumber(args[2]));
            ::Color color = vmExtractRaylibColor(args[3]);
            DrawCircle(x, y, radius, color);
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    return Value{obj};
}

// ---------------------------------------------------------------------------
// Window object builder
// ---------------------------------------------------------------------------

static Value vmBuildWindowObject(std::shared_ptr<VmUiWindow> win) {
    auto obj = std::make_shared<Map>();

    obj->entries["isOpen"] = Value{std::make_shared<VmNativeFunction>("isOpen", 0,
        [win](VM&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            if (win->open) {
                win->open = !WindowShouldClose();
            }
#endif
            return static_cast<bool>(win->open);
        })};

    obj->entries["close"] = Value{std::make_shared<VmNativeFunction>("close", 0,
        [win](VM&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            if (win->open) {
                CloseWindow();
                win->open = false;
            }
#endif
            win->open = false;
            return Nil{};
        })};

    obj->entries["beginDrawing"] = Value{std::make_shared<VmNativeFunction>("beginDrawing", 0,
        [](VM&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            BeginDrawing();
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    obj->entries["endDrawing"] = Value{std::make_shared<VmNativeFunction>("endDrawing", 0,
        [](VM&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            EndDrawing();
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    obj->entries["clear"] = Value{std::make_shared<VmNativeFunction>("clear", 1,
        [](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1) {
                throw std::runtime_error("win.clear() takes 1 argument.");
            }
#ifdef HAVE_RAYLIB
            ClearBackground(vmExtractRaylibColor(args[0]));
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    obj->entries["setTitle"] = Value{std::make_shared<VmNativeFunction>("setTitle", 1,
        [win](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1 || !std::holds_alternative<std::string>(args[0])) {
                throw std::runtime_error("win.setTitle() takes 1 string argument.");
            }
            win->title = std::get<std::string>(args[0]);
#ifdef HAVE_RAYLIB
            SetWindowTitle(win->title.c_str());
#endif
            return Nil{};
        })};

    obj->entries["getSize"] = Value{std::make_shared<VmNativeFunction>("getSize", 0,
        [win](VM&, const std::vector<Value>&) -> Value {
            auto m = std::make_shared<Map>();
#ifdef HAVE_RAYLIB
            win->width = GetScreenWidth();
            win->height = GetScreenHeight();
#endif
            m->entries["width"] = static_cast<double>(win->width);
            m->entries["height"] = static_cast<double>(win->height);
            return Value{m};
        })};

    obj->entries["getFps"] = Value{std::make_shared<VmNativeFunction>("getFps", 0,
        [](VM&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            return static_cast<double>(GetFPS());
#else
            return 0.0;
#endif
        })};

    obj->entries["setTargetFPS"] = Value{std::make_shared<VmNativeFunction>("setTargetFPS", 1,
        [](VM&, const std::vector<Value>& args) -> Value {
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

    obj->entries["getFrameTime"] = Value{std::make_shared<VmNativeFunction>("getFrameTime", 0,
        [](VM&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            return static_cast<double>(GetFrameTime());
#else
            return 0.0;
#endif
        })};

    obj->entries["drawText"] = Value{std::make_shared<VmNativeFunction>("drawText", 5,
        [](VM&, const std::vector<Value>& args) -> Value {
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
            ::Color color = vmExtractRaylibColor(args[4]);
            DrawText(text.c_str(), x, y, fontSize, color);
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    obj->entries["drawRect"] = Value{std::make_shared<VmNativeFunction>("drawRect", 5,
        [](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 5) {
                throw std::runtime_error("win.drawRect() takes 5 arguments (x, y, width, height, color).");
            }
#ifdef HAVE_RAYLIB
            int x = static_cast<int>(asNumber(args[0]));
            int y = static_cast<int>(asNumber(args[1]));
            int w = static_cast<int>(asNumber(args[2]));
            int h = static_cast<int>(asNumber(args[3]));
            ::Color color = vmExtractRaylibColor(args[4]);
            DrawRectangleLines(x, y, w, h, color);
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    obj->entries["fillRect"] = Value{std::make_shared<VmNativeFunction>("fillRect", 5,
        [](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 5) {
                throw std::runtime_error("win.fillRect() takes 5 arguments (x, y, width, height, color).");
            }
#ifdef HAVE_RAYLIB
            int x = static_cast<int>(asNumber(args[0]));
            int y = static_cast<int>(asNumber(args[1]));
            int w = static_cast<int>(asNumber(args[2]));
            int h = static_cast<int>(asNumber(args[3]));
            ::Color color = vmExtractRaylibColor(args[4]);
            DrawRectangle(x, y, w, h, color);
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    obj->entries["drawLine"] = Value{std::make_shared<VmNativeFunction>("drawLine", 6,
        [](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 6) {
                throw std::runtime_error("win.drawLine() takes 6 arguments (x1, y1, x2, y2, thickness, color).");
            }
#ifdef HAVE_RAYLIB
            int x1 = static_cast<int>(asNumber(args[0]));
            int y1 = static_cast<int>(asNumber(args[1]));
            int x2 = static_cast<int>(asNumber(args[2]));
            int y2 = static_cast<int>(asNumber(args[3]));
            float thickness = static_cast<float>(asNumber(args[4]));
            ::Color color = vmExtractRaylibColor(args[5]);
            DrawLineEx({static_cast<float>(x1), static_cast<float>(y1)},
                       {static_cast<float>(x2), static_cast<float>(y2)}, thickness, color);
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    obj->entries["drawCircle"] = Value{std::make_shared<VmNativeFunction>("drawCircle", 4,
        [](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 4) {
                throw std::runtime_error("win.drawCircle() takes 4 arguments (x, y, radius, color).");
            }
#ifdef HAVE_RAYLIB
            int x = static_cast<int>(asNumber(args[0]));
            int y = static_cast<int>(asNumber(args[1]));
            float radius = static_cast<float>(asNumber(args[2]));
            ::Color color = vmExtractRaylibColor(args[3]);
            DrawCircle(x, y, radius, color);
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    obj->entries["createPanel"] = Value{std::make_shared<VmNativeFunction>("createPanel", 4,
        [](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 4) {
                throw std::runtime_error(
                    "win.createPanel() takes 4 arguments (x, y, width, height).");
            }
            auto p = std::make_shared<VmUiPanel>();
            p->x = static_cast<int>(asNumber(args[0]));
            p->y = static_cast<int>(asNumber(args[1]));
            p->width = static_cast<int>(asNumber(args[2]));
            p->height = static_cast<int>(asNumber(args[3]));
            return vmBuildPanelObject(p);
        })};

    return Value{obj};
}

// ---------------------------------------------------------------------------
// ui.createWindow(title, width, height)
// ---------------------------------------------------------------------------
static Value vmNativeUiCreateWindow(VM& /*vm*/, const std::vector<Value>& args) {
    if (args.size() != 3) {
        throw std::runtime_error("ui.createWindow() takes 3 arguments (title, width, height).");
    }
    if (!std::holds_alternative<std::string>(args[0])) {
        throw std::runtime_error("ui.createWindow(): title must be a string.");
    }
    std::string title = std::get<std::string>(args[0]);
    int width = static_cast<int>(asNumber(args[1]));
    int height = static_cast<int>(asNumber(args[2]));
    auto win = std::make_shared<VmUiWindow>();
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
    return vmBuildWindowObject(win);
}

// ---------------------------------------------------------------------------
// Build the top-level ui module Map (VM version)
// ---------------------------------------------------------------------------
Value createVmUiModule(VM& /*vm*/) {
    auto module = std::make_shared<Map>();

    module->entries["createWindow"] =
        Value{std::make_shared<VmNativeFunction>("createWindow", 3, vmNativeUiCreateWindow)};

    module->entries["color"] = Value{std::make_shared<VmNativeFunction>("color", -1,
        [](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() < 3 || args.size() > 4) {
                throw std::runtime_error("ui.color() takes 3 or 4 arguments (r, g, b [, a]).");
            }
            int r = static_cast<int>(asNumber(args[0]));
            int g = static_cast<int>(asNumber(args[1]));
            int b = static_cast<int>(asNumber(args[2]));
            int a = (args.size() == 4) ? static_cast<int>(asNumber(args[3])) : 255;
            return vmMakeColorValue(r, g, b, a);
        })};

    module->entries["keyDown"] = Value{std::make_shared<VmNativeFunction>("keyDown", 1,
        [](VM&, const std::vector<Value>& args) -> Value {
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
        })};

    module->entries["keyPressed"] = Value{std::make_shared<VmNativeFunction>("keyPressed", 1,
        [](VM&, const std::vector<Value>& args) -> Value {
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
        })};

    module->entries["mouseDown"] = Value{std::make_shared<VmNativeFunction>("mouseDown", 1,
        [](VM&, const std::vector<Value>& args) -> Value {
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
        })};

    module->entries["mousePressed"] = Value{std::make_shared<VmNativeFunction>("mousePressed", 1,
        [](VM&, const std::vector<Value>& args) -> Value {
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
        })};

    module->entries["getMousePosition"] = Value{std::make_shared<VmNativeFunction>("getMousePosition", 0,
        [](VM&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            ::Vector2 pos = GetMousePosition();
            auto m = std::make_shared<Map>();
            m->entries["x"] = static_cast<double>(pos.x);
            m->entries["y"] = static_cast<double>(pos.y);
            return Value{m};
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
        })};

    module->entries["getMouseWheelMove"] = Value{std::make_shared<VmNativeFunction>("getMouseWheelMove", 0,
        [](VM&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            return static_cast<double>(GetMouseWheelMove());
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
        })};

    module->entries["getCharPressed"] = Value{std::make_shared<VmNativeFunction>("getCharPressed", 0,
        [](VM&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            return static_cast<double>(GetCharPressed());
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
