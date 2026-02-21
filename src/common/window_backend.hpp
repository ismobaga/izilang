#pragma once

#include "value.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <string>
#include <unordered_map>

// Shared SDL2 window state and helper functions used by both the tree-walking
// interpreter (native.cpp) and the bytecode VM (vm_native.cpp).
// Using C++17 inline variables to guarantee a single shared instance.

namespace izi {
namespace window_backend {

struct WindowEntry {
    SDL_Window*   window   = nullptr;
    SDL_Renderer* renderer = nullptr;
    bool          open     = true;
};

inline std::unordered_map<int, WindowEntry> g_windows;
inline int  g_nextWindowId    = 1;
inline bool g_sdlInitialized  = false;
inline bool g_ttfInitialized  = false;

// Font cache: maps font size -> open TTF_Font*. Empty string key means "no font available".
inline std::unordered_map<int, TTF_Font*> g_fontCache;
inline bool g_fontSearchDone  = false;  // true once we've found (or failed to find) a system font
inline std::string g_fontPath;          // empty if no font was found

inline void ensureSdlInit() {
    if (!g_sdlInitialized) {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            throw std::runtime_error(std::string("SDL_Init failed: ") + SDL_GetError());
        }
        g_sdlInitialized = true;
    }
}

inline void ensureTtfInit() {
    if (!g_ttfInitialized) {
        if (TTF_Init() != 0) {
            throw std::runtime_error(std::string("TTF_Init failed: ") + TTF_GetError());
        }
        g_ttfInitialized = true;
    }
}

// Find a usable TrueType font path, caching the result after the first call.
inline const std::string& findFontPath() {
    if (g_fontSearchDone) return g_fontPath;
    g_fontSearchDone = true;

    static const char* candidates[] = {
        // Linux (Debian/Ubuntu/Fedora/Arch)
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
        "/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf",
        // macOS
        "/System/Library/Fonts/Helvetica.ttc",
        "/System/Library/Fonts/Arial.ttf",
        "/Library/Fonts/Arial.ttf",
        // Windows
        "C:\\Windows\\Fonts\\arial.ttf",
        "C:\\Windows\\Fonts\\segoeui.ttf",
        "C:\\Windows\\Fonts\\calibri.ttf",
        nullptr
    };

    for (int i = 0; candidates[i] != nullptr; ++i) {
        TTF_Font* probe = TTF_OpenFont(candidates[i], 12);
        if (probe) {
            TTF_CloseFont(probe);
            g_fontPath = candidates[i];
            return g_fontPath;
        }
    }

    // No font found — emit a one-time warning to stderr
    std::cerr << "[window] Warning: no TrueType font found; drawText() calls will be no-ops.\n"
              << "         Install fonts (e.g. fonts-dejavu-core) to enable text rendering.\n";
    return g_fontPath;  // empty string
}

// Get (or open) a cached font for the given size. Returns nullptr if unavailable.
inline TTF_Font* getCachedFont(int fontSize) {
    ensureTtfInit();
    const std::string& path = findFontPath();
    if (path.empty()) return nullptr;

    auto it = g_fontCache.find(fontSize);
    if (it != g_fontCache.end()) return it->second;

    TTF_Font* font = TTF_OpenFont(path.c_str(), fontSize);
    g_fontCache[fontSize] = font;  // cache even if nullptr
    return font;
}

inline WindowEntry& getWindowEntry(int id) {
    auto it = g_windows.find(id);
    if (it == g_windows.end()) {
        throw std::runtime_error("window: invalid window id " + std::to_string(id));
    }
    if (!it->second.open) {
        throw std::runtime_error("window: window " + std::to_string(id) + " has been destroyed");
    }
    return it->second;
}

inline uint8_t toChannel(const Value& v, const char* name) {
    if (!std::holds_alternative<double>(v)) {
        throw std::runtime_error(std::string(name) + " must be a number");
    }
    double d = std::get<double>(v);
    if (d < 0 || d > 255) {
        throw std::runtime_error(std::string(name) + " must be 0-255");
    }
    return static_cast<uint8_t>(d);
}

inline Value windowCreate(const std::vector<Value>& args) {
    if (args.size() != 3) {
        throw std::runtime_error("window.create(title, width, height) requires 3 arguments.");
    }
    if (!std::holds_alternative<std::string>(args[0])) {
        throw std::runtime_error("window.create: title must be a string.");
    }
    if (!std::holds_alternative<double>(args[1]) || !std::holds_alternative<double>(args[2])) {
        throw std::runtime_error("window.create: width and height must be numbers.");
    }

    ensureSdlInit();

    const std::string& title = std::get<std::string>(args[0]);
    int w = static_cast<int>(std::get<double>(args[1]));
    int h = static_cast<int>(std::get<double>(args[2]));

    SDL_Window* win = SDL_CreateWindow(
        title.c_str(),
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        w, h,
        SDL_WINDOW_SHOWN
    );
    if (!win) {
        throw std::runtime_error(std::string("window.create: SDL_CreateWindow failed: ") + SDL_GetError());
    }
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ren) {
        SDL_DestroyWindow(win);
        throw std::runtime_error(std::string("window.create: SDL_CreateRenderer failed: ") + SDL_GetError());
    }

    int id = g_nextWindowId++;
    g_windows[id] = {win, ren, true};
    return static_cast<double>(id);
}

inline Value windowDestroy(const std::vector<Value>& args) {
    if (args.size() != 1 || !std::holds_alternative<double>(args[0])) {
        throw std::runtime_error("window.destroy(handle) requires 1 numeric argument.");
    }
    int id = static_cast<int>(std::get<double>(args[0]));
    auto it = g_windows.find(id);
    if (it != g_windows.end() && it->second.open) {
        SDL_DestroyRenderer(it->second.renderer);
        SDL_DestroyWindow(it->second.window);
        it->second.open     = false;
        it->second.renderer = nullptr;
        it->second.window   = nullptr;
    }
    return Nil{};
}

inline Value windowClear(const std::vector<Value>& args) {
    if (args.empty() || !std::holds_alternative<double>(args[0])) {
        throw std::runtime_error("window.clear(handle [, r, g, b]) requires at least 1 numeric argument.");
    }
    int id = static_cast<int>(std::get<double>(args[0]));
    WindowEntry& entry = getWindowEntry(id);

    uint8_t r = 0, g = 0, b = 0, a = 255;
    if (args.size() >= 4) {
        r = toChannel(args[1], "r");
        g = toChannel(args[2], "g");
        b = toChannel(args[3], "b");
    }
    SDL_SetRenderDrawColor(entry.renderer, r, g, b, a);
    SDL_RenderClear(entry.renderer);
    return Nil{};
}

inline Value windowPresent(const std::vector<Value>& args) {
    if (args.size() != 1 || !std::holds_alternative<double>(args[0])) {
        throw std::runtime_error("window.present(handle) requires 1 numeric argument.");
    }
    int id = static_cast<int>(std::get<double>(args[0]));
    WindowEntry& entry = getWindowEntry(id);
    SDL_RenderPresent(entry.renderer);
    return Nil{};
}

inline Value windowPollEvent(const std::vector<Value>& /*args*/) {
    auto result = std::make_shared<Map>();
    SDL_Event e;
    if (!SDL_PollEvent(&e)) {
        result->entries["type"] = std::string("none");
        return Value{result};
    }

    switch (e.type) {
        case SDL_QUIT:
            result->entries["type"] = std::string("quit");
            for (auto& [id, entry] : g_windows) {
                entry.open = false;
            }
            break;

        case SDL_KEYDOWN:
        case SDL_KEYUP: {
            result->entries["type"] = std::string(e.type == SDL_KEYDOWN ? "keydown" : "keyup");
            const char* keyName = SDL_GetKeyName(e.key.keysym.sym);
            result->entries["key"]      = std::string(keyName ? keyName : "");
            result->entries["scancode"] = static_cast<double>(e.key.keysym.scancode);
            result->entries["repeat"]   = static_cast<bool>(e.key.repeat != 0);
            break;
        }

        case SDL_MOUSEMOTION:
            result->entries["type"] = std::string("mousemove");
            result->entries["x"]    = static_cast<double>(e.motion.x);
            result->entries["y"]    = static_cast<double>(e.motion.y);
            result->entries["dx"]   = static_cast<double>(e.motion.xrel);
            result->entries["dy"]   = static_cast<double>(e.motion.yrel);
            break;

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            result->entries["type"]   = std::string(e.type == SDL_MOUSEBUTTONDOWN ? "mousedown" : "mouseup");
            result->entries["x"]      = static_cast<double>(e.button.x);
            result->entries["y"]      = static_cast<double>(e.button.y);
            result->entries["button"] = static_cast<double>(e.button.button);
            break;

        case SDL_MOUSEWHEEL:
            result->entries["type"] = std::string("mousewheel");
            result->entries["x"]    = static_cast<double>(e.wheel.x);
            result->entries["y"]    = static_cast<double>(e.wheel.y);
            break;

        case SDL_WINDOWEVENT:
            result->entries["type"]  = std::string("window");
            result->entries["event"] = static_cast<double>(e.window.event);
            if (e.window.event == SDL_WINDOWEVENT_CLOSE) {
                result->entries["type"] = std::string("quit");
            }
            break;

        default:
            result->entries["type"] = std::string("other");
            break;
    }
    return Value{result};
}

inline Value windowDrawRect(const std::vector<Value>& args) {
    // drawRect(handle, x, y, w, h [, r, g, b [, a]])
    if (args.size() < 5) {
        throw std::runtime_error("window.drawRect(handle, x, y, w, h [, r, g, b, a]) requires at least 5 arguments.");
    }
    int id = static_cast<int>(std::get<double>(args[0]));
    WindowEntry& entry = getWindowEntry(id);

    SDL_Rect rect;
    rect.x = static_cast<int>(std::get<double>(args[1]));
    rect.y = static_cast<int>(std::get<double>(args[2]));
    rect.w = static_cast<int>(std::get<double>(args[3]));
    rect.h = static_cast<int>(std::get<double>(args[4]));

    uint8_t r = 255, g = 255, b = 255, a = 255;
    if (args.size() >= 8) {
        r = toChannel(args[5], "r");
        g = toChannel(args[6], "g");
        b = toChannel(args[7], "b");
    }
    if (args.size() >= 9) {
        a = toChannel(args[8], "a");
    }

    SDL_SetRenderDrawColor(entry.renderer, r, g, b, a);
    SDL_RenderFillRect(entry.renderer, &rect);
    return Nil{};
}

inline Value windowDrawLine(const std::vector<Value>& args) {
    // drawLine(handle, x1, y1, x2, y2 [, r, g, b, a])
    if (args.size() < 5) {
        throw std::runtime_error("window.drawLine(handle, x1, y1, x2, y2 [, r, g, b, a]) requires at least 5 arguments.");
    }
    int id = static_cast<int>(std::get<double>(args[0]));
    WindowEntry& entry = getWindowEntry(id);

    int x1 = static_cast<int>(std::get<double>(args[1]));
    int y1 = static_cast<int>(std::get<double>(args[2]));
    int x2 = static_cast<int>(std::get<double>(args[3]));
    int y2 = static_cast<int>(std::get<double>(args[4]));

    uint8_t r = 255, g = 255, b = 255, a = 255;
    if (args.size() >= 8) {
        r = toChannel(args[5], "r");
        g = toChannel(args[6], "g");
        b = toChannel(args[7], "b");
    }
    if (args.size() >= 9) {
        a = toChannel(args[8], "a");
    }

    SDL_SetRenderDrawColor(entry.renderer, r, g, b, a);
    SDL_RenderDrawLine(entry.renderer, x1, y1, x2, y2);
    return Nil{};
}

inline Value windowDrawText(const std::vector<Value>& args) {
    // drawText(handle, text, x, y [, r, g, b [, fontSize]])
    if (args.size() < 4) {
        throw std::runtime_error("window.drawText(handle, text, x, y [, r, g, b, fontSize]) requires at least 4 arguments.");
    }
    int id = static_cast<int>(std::get<double>(args[0]));
    WindowEntry& entry = getWindowEntry(id);

    if (!std::holds_alternative<std::string>(args[1])) {
        throw std::runtime_error("window.drawText: text must be a string.");
    }
    const std::string& text = std::get<std::string>(args[1]);
    int x = static_cast<int>(std::get<double>(args[2]));
    int y = static_cast<int>(std::get<double>(args[3]));

    uint8_t r = 255, g = 255, b = 255;
    if (args.size() >= 7) {
        r = toChannel(args[4], "r");
        g = toChannel(args[5], "g");
        b = toChannel(args[6], "b");
    }
    int fontSize = 16;
    if (args.size() >= 8 && std::holds_alternative<double>(args[7])) {
        fontSize = static_cast<int>(std::get<double>(args[7]));
        if (fontSize < 1) fontSize = 1;
    }

    ensureTtfInit();

    TTF_Font* font = getCachedFont(fontSize);
    if (!font) {
        return Nil{};
    }

    SDL_Color color = {r, g, b, 255};
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.empty() ? " " : text.c_str(), color);
    // Note: font is NOT closed here — it is owned by the cache.
    if (!surface) {
        return Nil{};
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(entry.renderer, surface);
    if (texture) {
        SDL_Rect dst = {x, y, surface->w, surface->h};
        SDL_RenderCopy(entry.renderer, texture, nullptr, &dst);
        SDL_DestroyTexture(texture);
    }
    SDL_FreeSurface(surface);
    return Nil{};
}

inline Value windowSetTitle(const std::vector<Value>& args) {
    if (args.size() != 2 || !std::holds_alternative<double>(args[0]) ||
        !std::holds_alternative<std::string>(args[1])) {
        throw std::runtime_error("window.setTitle(handle, title) requires a handle and a string.");
    }
    int id = static_cast<int>(std::get<double>(args[0]));
    WindowEntry& entry = getWindowEntry(id);
    SDL_SetWindowTitle(entry.window, std::get<std::string>(args[1]).c_str());
    return Nil{};
}

inline Value windowGetSize(const std::vector<Value>& args) {
    if (args.size() != 1 || !std::holds_alternative<double>(args[0])) {
        throw std::runtime_error("window.getSize(handle) requires 1 numeric argument.");
    }
    int id = static_cast<int>(std::get<double>(args[0]));
    WindowEntry& entry = getWindowEntry(id);

    int w = 0, h = 0;
    SDL_GetWindowSize(entry.window, &w, &h);

    auto result = std::make_shared<Map>();
    result->entries["width"]  = static_cast<double>(w);
    result->entries["height"] = static_cast<double>(h);
    return Value{result};
}

inline Value windowIsOpen(const std::vector<Value>& args) {
    if (args.size() != 1 || !std::holds_alternative<double>(args[0])) {
        throw std::runtime_error("window.isOpen(handle) requires 1 numeric argument.");
    }
    int id = static_cast<int>(std::get<double>(args[0]));
    auto it = g_windows.find(id);
    if (it == g_windows.end()) return false;
    return it->second.open;
}

} // namespace window_backend
} // namespace izi
