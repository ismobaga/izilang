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

    obj->entries["drawCircleLines"] = Value{std::make_shared<VmNativeFunction>("drawCircleLines", 4,
        [panel](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 4) {
                throw std::runtime_error(
                    "panel.drawCircleLines() takes 4 arguments (x, y, radius, color).");
            }
#ifdef HAVE_RAYLIB
            int x = static_cast<int>(asNumber(args[0])) + panel->x;
            int y = static_cast<int>(asNumber(args[1])) + panel->y;
            float radius = static_cast<float>(asNumber(args[2]));
            ::Color color = vmExtractRaylibColor(args[3]);
            DrawCircleLines(x, y, radius, color);
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    obj->entries["fillRectRounded"] = Value{std::make_shared<VmNativeFunction>("fillRectRounded", 6,
        [panel](VM&, const std::vector<Value>& args) -> Value {
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
            ::Color color = vmExtractRaylibColor(args[5]);
            DrawRectangleRounded({x, y, w, h}, roundness, 8, color);
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    obj->entries["drawRectRounded"] = Value{std::make_shared<VmNativeFunction>("drawRectRounded", 6,
        [panel](VM&, const std::vector<Value>& args) -> Value {
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
            ::Color color = vmExtractRaylibColor(args[5]);
            DrawRectangleRoundedLines({x, y, w, h}, roundness, 8, color);
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    obj->entries["measureText"] = Value{std::make_shared<VmNativeFunction>("measureText", 2,
        [](VM&, const std::vector<Value>& args) -> Value {
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

    obj->entries["drawTriangle"] = Value{std::make_shared<VmNativeFunction>("drawTriangle", 7,
        [panel](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 7) {
                throw std::runtime_error(
                    "panel.drawTriangle() takes 7 arguments (x1, y1, x2, y2, x3, y3, color).");
            }
#ifdef HAVE_RAYLIB
            float x1 = static_cast<float>(asNumber(args[0])) + static_cast<float>(panel->x);
            float y1 = static_cast<float>(asNumber(args[1])) + static_cast<float>(panel->y);
            float x2 = static_cast<float>(asNumber(args[2])) + static_cast<float>(panel->x);
            float y2 = static_cast<float>(asNumber(args[3])) + static_cast<float>(panel->y);
            float x3 = static_cast<float>(asNumber(args[4])) + static_cast<float>(panel->x);
            float y3 = static_cast<float>(asNumber(args[5])) + static_cast<float>(panel->y);
            ::Color color = vmExtractRaylibColor(args[6]);
            DrawTriangle({x1, y1}, {x2, y2}, {x3, y3}, color);
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    obj->entries["drawTriangleLines"] = Value{std::make_shared<VmNativeFunction>("drawTriangleLines", 7,
        [panel](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 7) {
                throw std::runtime_error(
                    "panel.drawTriangleLines() takes 7 arguments (x1, y1, x2, y2, x3, y3, color).");
            }
#ifdef HAVE_RAYLIB
            float x1 = static_cast<float>(asNumber(args[0])) + static_cast<float>(panel->x);
            float y1 = static_cast<float>(asNumber(args[1])) + static_cast<float>(panel->y);
            float x2 = static_cast<float>(asNumber(args[2])) + static_cast<float>(panel->x);
            float y2 = static_cast<float>(asNumber(args[3])) + static_cast<float>(panel->y);
            float x3 = static_cast<float>(asNumber(args[4])) + static_cast<float>(panel->x);
            float y3 = static_cast<float>(asNumber(args[5])) + static_cast<float>(panel->y);
            ::Color color = vmExtractRaylibColor(args[6]);
            DrawTriangleLines({x1, y1}, {x2, y2}, {x3, y3}, color);
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    obj->entries["fillRectGradientV"] = Value{std::make_shared<VmNativeFunction>("fillRectGradientV", 6,
        [panel](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 6) {
                throw std::runtime_error(
                    "panel.fillRectGradientV() takes 6 arguments (x, y, width, height, colorTop, colorBottom).");
            }
#ifdef HAVE_RAYLIB
            int x = static_cast<int>(asNumber(args[0])) + panel->x;
            int y = static_cast<int>(asNumber(args[1])) + panel->y;
            int w = static_cast<int>(asNumber(args[2]));
            int h = static_cast<int>(asNumber(args[3]));
            ::Color top = vmExtractRaylibColor(args[4]);
            ::Color bottom = vmExtractRaylibColor(args[5]);
            DrawRectangleGradientV(x, y, w, h, top, bottom);
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    obj->entries["fillRectGradientH"] = Value{std::make_shared<VmNativeFunction>("fillRectGradientH", 6,
        [panel](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 6) {
                throw std::runtime_error(
                    "panel.fillRectGradientH() takes 6 arguments (x, y, width, height, colorLeft, colorRight).");
            }
#ifdef HAVE_RAYLIB
            int x = static_cast<int>(asNumber(args[0])) + panel->x;
            int y = static_cast<int>(asNumber(args[1])) + panel->y;
            int w = static_cast<int>(asNumber(args[2]));
            int h = static_cast<int>(asNumber(args[3]));
            ::Color left = vmExtractRaylibColor(args[4]);
            ::Color right = vmExtractRaylibColor(args[5]);
            DrawRectangleGradientH(x, y, w, h, left, right);
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

    obj->entries["drawCircleLines"] = Value{std::make_shared<VmNativeFunction>("drawCircleLines", 4,
        [](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 4) {
                throw std::runtime_error("win.drawCircleLines() takes 4 arguments (x, y, radius, color).");
            }
#ifdef HAVE_RAYLIB
            int x = static_cast<int>(asNumber(args[0]));
            int y = static_cast<int>(asNumber(args[1]));
            float radius = static_cast<float>(asNumber(args[2]));
            ::Color color = vmExtractRaylibColor(args[3]);
            DrawCircleLines(x, y, radius, color);
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    obj->entries["fillRectRounded"] = Value{std::make_shared<VmNativeFunction>("fillRectRounded", 6,
        [](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 6) {
                throw std::runtime_error("win.fillRectRounded() takes 6 arguments (x, y, width, height, roundness, color).");
            }
#ifdef HAVE_RAYLIB
            float x = static_cast<float>(asNumber(args[0]));
            float y = static_cast<float>(asNumber(args[1]));
            float w = static_cast<float>(asNumber(args[2]));
            float h = static_cast<float>(asNumber(args[3]));
            float roundness = static_cast<float>(asNumber(args[4]));
            ::Color color = vmExtractRaylibColor(args[5]);
            DrawRectangleRounded({x, y, w, h}, roundness, 8, color);
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    obj->entries["drawRectRounded"] = Value{std::make_shared<VmNativeFunction>("drawRectRounded", 6,
        [](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 6) {
                throw std::runtime_error("win.drawRectRounded() takes 6 arguments (x, y, width, height, roundness, color).");
            }
#ifdef HAVE_RAYLIB
            float x = static_cast<float>(asNumber(args[0]));
            float y = static_cast<float>(asNumber(args[1]));
            float w = static_cast<float>(asNumber(args[2]));
            float h = static_cast<float>(asNumber(args[3]));
            float roundness = static_cast<float>(asNumber(args[4]));
            ::Color color = vmExtractRaylibColor(args[5]);
            DrawRectangleRoundedLines({x, y, w, h}, roundness, 8, color);
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    obj->entries["measureText"] = Value{std::make_shared<VmNativeFunction>("measureText", 2,
        [](VM&, const std::vector<Value>& args) -> Value {
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

    obj->entries["toggleFullscreen"] = Value{std::make_shared<VmNativeFunction>("toggleFullscreen", 0,
        [](VM&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            ToggleFullscreen();
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    obj->entries["isWindowFocused"] = Value{std::make_shared<VmNativeFunction>("isWindowFocused", 0,
        [](VM&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            return static_cast<bool>(IsWindowFocused());
#else
            return false;
#endif
        })};

    obj->entries["setWindowMinSize"] = Value{std::make_shared<VmNativeFunction>("setWindowMinSize", 2,
        [](VM&, const std::vector<Value>& args) -> Value {
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

    obj->entries["drawTriangle"] = Value{std::make_shared<VmNativeFunction>("drawTriangle", 7,
        [](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 7) {
                throw std::runtime_error(
                    "win.drawTriangle() takes 7 arguments (x1, y1, x2, y2, x3, y3, color).");
            }
#ifdef HAVE_RAYLIB
            float x1 = static_cast<float>(asNumber(args[0]));
            float y1 = static_cast<float>(asNumber(args[1]));
            float x2 = static_cast<float>(asNumber(args[2]));
            float y2 = static_cast<float>(asNumber(args[3]));
            float x3 = static_cast<float>(asNumber(args[4]));
            float y3 = static_cast<float>(asNumber(args[5]));
            ::Color color = vmExtractRaylibColor(args[6]);
            DrawTriangle({x1, y1}, {x2, y2}, {x3, y3}, color);
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    obj->entries["drawTriangleLines"] = Value{std::make_shared<VmNativeFunction>("drawTriangleLines", 7,
        [](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 7) {
                throw std::runtime_error(
                    "win.drawTriangleLines() takes 7 arguments (x1, y1, x2, y2, x3, y3, color).");
            }
#ifdef HAVE_RAYLIB
            float x1 = static_cast<float>(asNumber(args[0]));
            float y1 = static_cast<float>(asNumber(args[1]));
            float x2 = static_cast<float>(asNumber(args[2]));
            float y2 = static_cast<float>(asNumber(args[3]));
            float x3 = static_cast<float>(asNumber(args[4]));
            float y3 = static_cast<float>(asNumber(args[5]));
            ::Color color = vmExtractRaylibColor(args[6]);
            DrawTriangleLines({x1, y1}, {x2, y2}, {x3, y3}, color);
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    obj->entries["fillRectGradientV"] = Value{std::make_shared<VmNativeFunction>("fillRectGradientV", 6,
        [](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 6) {
                throw std::runtime_error(
                    "win.fillRectGradientV() takes 6 arguments (x, y, width, height, colorTop, colorBottom).");
            }
#ifdef HAVE_RAYLIB
            int x = static_cast<int>(asNumber(args[0]));
            int y = static_cast<int>(asNumber(args[1]));
            int w = static_cast<int>(asNumber(args[2]));
            int h = static_cast<int>(asNumber(args[3]));
            ::Color top = vmExtractRaylibColor(args[4]);
            ::Color bottom = vmExtractRaylibColor(args[5]);
            DrawRectangleGradientV(x, y, w, h, top, bottom);
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    obj->entries["fillRectGradientH"] = Value{std::make_shared<VmNativeFunction>("fillRectGradientH", 6,
        [](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 6) {
                throw std::runtime_error(
                    "win.fillRectGradientH() takes 6 arguments (x, y, width, height, colorLeft, colorRight).");
            }
#ifdef HAVE_RAYLIB
            int x = static_cast<int>(asNumber(args[0]));
            int y = static_cast<int>(asNumber(args[1]));
            int w = static_cast<int>(asNumber(args[2]));
            int h = static_cast<int>(asNumber(args[3]));
            ::Color left = vmExtractRaylibColor(args[4]);
            ::Color right = vmExtractRaylibColor(args[5]);
            DrawRectangleGradientH(x, y, w, h, left, right);
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    obj->entries["drawFPS"] = Value{std::make_shared<VmNativeFunction>("drawFPS", 2,
        [](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 2) {
                throw std::runtime_error("win.drawFPS() takes 2 arguments (x, y).");
            }
#ifdef HAVE_RAYLIB
            DrawFPS(static_cast<int>(asNumber(args[0])), static_cast<int>(asNumber(args[1])));
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    return Value{obj};
}

// ---------------------------------------------------------------------------
// Camera2D state (VM version)
// ---------------------------------------------------------------------------
struct VmUiCamera2D {
    float offsetX = 0.0f, offsetY = 0.0f;
    float targetX = 0.0f, targetY = 0.0f;
    float rotation = 0.0f;
    float zoom = 1.0f;
};

static Value vmBuildCamera2DObject(std::shared_ptr<VmUiCamera2D> cam) {
    auto obj = std::make_shared<Map>();

    obj->entries["setTarget"] = Value{std::make_shared<VmNativeFunction>("setTarget", 2,
        [cam](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 2) {
                throw std::runtime_error("camera.setTarget() takes 2 arguments (x, y).");
            }
            cam->targetX = static_cast<float>(asNumber(args[0]));
            cam->targetY = static_cast<float>(asNumber(args[1]));
            return Nil{};
        })};

    obj->entries["setOffset"] = Value{std::make_shared<VmNativeFunction>("setOffset", 2,
        [cam](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 2) {
                throw std::runtime_error("camera.setOffset() takes 2 arguments (x, y).");
            }
            cam->offsetX = static_cast<float>(asNumber(args[0]));
            cam->offsetY = static_cast<float>(asNumber(args[1]));
            return Nil{};
        })};

    obj->entries["setRotation"] = Value{std::make_shared<VmNativeFunction>("setRotation", 1,
        [cam](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1) {
                throw std::runtime_error("camera.setRotation() takes 1 argument (degrees).");
            }
            // Rotation is in degrees, matching raylib's Camera2D convention
            cam->rotation = static_cast<float>(asNumber(args[0]));
            return Nil{};
        })};

    obj->entries["setZoom"] = Value{std::make_shared<VmNativeFunction>("setZoom", 1,
        [cam](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1) {
                throw std::runtime_error("camera.setZoom() takes 1 argument (zoom).");
            }
            cam->zoom = static_cast<float>(asNumber(args[0]));
            return Nil{};
        })};

    obj->entries["beginMode"] = Value{std::make_shared<VmNativeFunction>("beginMode", 0,
        [cam](VM&, const std::vector<Value>&) -> Value {
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

    obj->entries["endMode"] = Value{std::make_shared<VmNativeFunction>("endMode", 0,
        [](VM&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            EndMode2D();
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    obj->entries["getWorldToScreen"] = Value{std::make_shared<VmNativeFunction>("getWorldToScreen", 2,
        [cam](VM&, const std::vector<Value>& args) -> Value {
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

    obj->entries["getScreenToWorld"] = Value{std::make_shared<VmNativeFunction>("getScreenToWorld", 2,
        [cam](VM&, const std::vector<Value>& args) -> Value {
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
// Texture wrapper (VM version)
// ---------------------------------------------------------------------------
#ifdef HAVE_RAYLIB
struct VmUiTexture {
    ::Texture2D texture{};
    bool loaded = false;
    ~VmUiTexture() {
        if (loaded) {
            UnloadTexture(texture);
            loaded = false;
        }
    }
};

static Value vmBuildTextureObject(std::shared_ptr<VmUiTexture> tex) {
    auto obj = std::make_shared<Map>();

    obj->entries["getWidth"] = Value{std::make_shared<VmNativeFunction>("getWidth", 0,
        [tex](VM&, const std::vector<Value>&) -> Value {
            return tex->loaded ? static_cast<double>(tex->texture.width) : 0.0;
        })};

    obj->entries["getHeight"] = Value{std::make_shared<VmNativeFunction>("getHeight", 0,
        [tex](VM&, const std::vector<Value>&) -> Value {
            return tex->loaded ? static_cast<double>(tex->texture.height) : 0.0;
        })};

    obj->entries["draw"] = Value{std::make_shared<VmNativeFunction>("draw", 3,
        [tex](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 3) {
                throw std::runtime_error("texture.draw() takes 3 arguments (x, y, tint).");
            }
            if (tex->loaded) {
                int x = static_cast<int>(asNumber(args[0]));
                int y = static_cast<int>(asNumber(args[1]));
                ::Color tint = vmExtractRaylibColor(args[2]);
                DrawTexture(tex->texture, x, y, tint);
            }
            return Nil{};
        })};

    obj->entries["drawEx"] = Value{std::make_shared<VmNativeFunction>("drawEx", 5,
        [tex](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 5) {
                throw std::runtime_error(
                    "texture.drawEx() takes 5 arguments (x, y, rotation, scale, tint).");
            }
            if (tex->loaded) {
                float x = static_cast<float>(asNumber(args[0]));
                float y = static_cast<float>(asNumber(args[1]));
                float rotation = static_cast<float>(asNumber(args[2]));
                float scale = static_cast<float>(asNumber(args[3]));
                ::Color tint = vmExtractRaylibColor(args[4]);
                DrawTextureEx(tex->texture, {x, y}, rotation, scale, tint);
            }
            return Nil{};
        })};

    obj->entries["drawRec"] = Value{std::make_shared<VmNativeFunction>("drawRec", 4,
        [tex](VM&, const std::vector<Value>& args) -> Value {
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
                ::Color tint = vmExtractRaylibColor(args[3]);
                DrawTextureRec(tex->texture, src, pos, tint);
            }
            return Nil{};
        })};

    obj->entries["unload"] = Value{std::make_shared<VmNativeFunction>("unload", 0,
        [tex](VM&, const std::vector<Value>&) -> Value {
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

    // Camera 2D
    module->entries["createCamera2D"] = Value{std::make_shared<VmNativeFunction>("createCamera2D", 6,
        [](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 6) {
                throw std::runtime_error(
                    "ui.createCamera2D() takes 6 arguments (offsetX, offsetY, targetX, targetY, rotation, zoom).");
            }
            auto cam = std::make_shared<VmUiCamera2D>();
            cam->offsetX  = static_cast<float>(asNumber(args[0]));
            cam->offsetY  = static_cast<float>(asNumber(args[1]));
            cam->targetX  = static_cast<float>(asNumber(args[2]));
            cam->targetY  = static_cast<float>(asNumber(args[3]));
            cam->rotation = static_cast<float>(asNumber(args[4]));
            cam->zoom     = static_cast<float>(asNumber(args[5]));
            return vmBuildCamera2DObject(cam);
        })};

    // Texture loading
    module->entries["loadTexture"] = Value{std::make_shared<VmNativeFunction>("loadTexture", 1,
        [](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1 || !std::holds_alternative<std::string>(args[0])) {
                throw std::runtime_error("ui.loadTexture() takes 1 string argument (path).");
            }
#ifdef HAVE_RAYLIB
            const std::string& path = std::get<std::string>(args[0]);
            auto tex = std::make_shared<VmUiTexture>();
            tex->texture = LoadTexture(path.c_str());
            tex->loaded = true;
            return vmBuildTextureObject(tex);
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
        })};


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

    module->entries["getTime"] = Value{std::make_shared<VmNativeFunction>("getTime", 0,
        [](VM&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            return static_cast<double>(GetTime());
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
        })};

    module->entries["keyReleased"] = Value{std::make_shared<VmNativeFunction>("keyReleased", 1,
        [](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1) {
                throw std::runtime_error("ui.keyReleased() takes 1 argument.");
            }
#ifdef HAVE_RAYLIB
            int key = static_cast<int>(asNumber(args[0]));
            return static_cast<bool>(IsKeyReleased(key));
#else
            (void)args;
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
        })};

    module->entries["mouseReleased"] = Value{std::make_shared<VmNativeFunction>("mouseReleased", 1,
        [](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1) {
                throw std::runtime_error("ui.mouseReleased() takes 1 argument.");
            }
#ifdef HAVE_RAYLIB
            int btn = static_cast<int>(asNumber(args[0]));
            return static_cast<bool>(IsMouseButtonReleased(btn));
#else
            (void)args;
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
        })};

    module->entries["getMouseDelta"] = Value{std::make_shared<VmNativeFunction>("getMouseDelta", 0,
        [](VM&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            ::Vector2 delta = GetMouseDelta();
            auto m = std::make_shared<Map>();
            m->entries["x"] = static_cast<double>(delta.x);
            m->entries["y"] = static_cast<double>(delta.y);
            return Value{m};
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
        })};

    module->entries["hideCursor"] = Value{std::make_shared<VmNativeFunction>("hideCursor", 0,
        [](VM&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            HideCursor();
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    module->entries["showCursor"] = Value{std::make_shared<VmNativeFunction>("showCursor", 0,
        [](VM&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            ShowCursor();
#else
            throw std::runtime_error("ui module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    module->entries["isCursorOnScreen"] = Value{std::make_shared<VmNativeFunction>("isCursorOnScreen", 0,
        [](VM&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            return static_cast<bool>(IsCursorOnScreen());
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
