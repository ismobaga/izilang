#include "vm_native_image.hpp"
#include "vm_native.hpp"
#include "vm.hpp"
#include <algorithm>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#ifdef HAVE_RAYLIB
#include <raylib.h>
#else
#include "../vendor/stb_image.h"
#include "../vendor/stb_image_write.h"
#include "../vendor/stb_image_resize2.h"
#endif

namespace izi {

// ---------------------------------------------------------------------------
// Image wrapper
// ---------------------------------------------------------------------------
#ifdef HAVE_RAYLIB
struct VmImageHandle {
    ::Image image{};
    bool loaded = false;
    ~VmImageHandle() {
        if (loaded) {
            UnloadImage(image);
            loaded = false;
        }
    }
};
#else
// stb_image-backed image handle (always RGBA, 4 channels)
struct VmImageHandle {
    std::vector<unsigned char> pixels;
    int width = 0;
    int height = 0;
    int channels = 4;
    bool loaded = false;
};

static std::vector<unsigned char> vmRotatePixels90CW(
        const std::vector<unsigned char>& src, int w, int h, int ch) {
    std::vector<unsigned char> dst(w * h * ch);
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int srcIdx = (y * w + x) * ch;
            int dstIdx = (x * h + (h - 1 - y)) * ch;
            std::memcpy(&dst[dstIdx], &src[srcIdx], ch);
        }
    }
    return dst;
}

static std::vector<unsigned char> vmBlurPixels(
        const std::vector<unsigned char>& src, int w, int h, int ch, int radius) {
    if (radius <= 0) return src;
    std::vector<unsigned char> dst(w * h * ch);
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int sum[4] = {0, 0, 0, 0};
            int count = 0;
            for (int ky = -radius; ky <= radius; ky++) {
                for (int kx = -radius; kx <= radius; kx++) {
                    int ny = y + ky, nx = x + kx;
                    if (ny >= 0 && ny < h && nx >= 0 && nx < w) {
                        int idx = (ny * w + nx) * ch;
                        for (int c = 0; c < ch; c++) sum[c] += src[idx + c];
                        count++;
                    }
                }
            }
            int outIdx = (y * w + x) * ch;
            for (int c = 0; c < ch; c++)
                dst[outIdx + c] = static_cast<unsigned char>(sum[c] / count);
        }
    }
    return dst;
}

static void vmSaveImageToFile(
        const std::string& path, const unsigned char* data, int w, int h, int ch) {
    auto dotPos = path.rfind('.');
    if (dotPos == std::string::npos)
        throw std::runtime_error("image.save(): cannot determine format from path '" + path + "'.");
    std::string ext = path.substr(dotPos);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    int ok = 0;
    if (ext == ".png") {
        ok = stbi_write_png(path.c_str(), w, h, ch, data, w * ch);
    } else if (ext == ".jpg" || ext == ".jpeg") {
        ok = stbi_write_jpg(path.c_str(), w, h, ch, data, 90);
    } else if (ext == ".bmp") {
        ok = stbi_write_bmp(path.c_str(), w, h, ch, data);
    } else {
        throw std::runtime_error(
            "image.save(): unsupported format '" + ext + "'. Use .png, .jpg, or .bmp.");
    }
    if (!ok)
        throw std::runtime_error("image.save(): failed to write image to '" + path + "'.");
}
#endif  // !HAVE_RAYLIB

// ---------------------------------------------------------------------------
// Build an image object from a loaded image handle (VM version)
// ---------------------------------------------------------------------------
static Value vmBuildImageObject(std::shared_ptr<VmImageHandle> handle);

#ifdef HAVE_RAYLIB
static Value vmBuildImageObject(std::shared_ptr<VmImageHandle> handle) {
    auto obj = std::make_shared<Map>();

    obj->entries["getWidth"] = Value{std::make_shared<VmNativeFunction>("getWidth", 0,
        [handle](VM&, const std::vector<Value>&) -> Value {
            if (!handle->loaded) return 0.0;
            return static_cast<double>(handle->image.width);
        })};

    obj->entries["getHeight"] = Value{std::make_shared<VmNativeFunction>("getHeight", 0,
        [handle](VM&, const std::vector<Value>&) -> Value {
            if (!handle->loaded) return 0.0;
            return static_cast<double>(handle->image.height);
        })};

    obj->entries["resize"] = Value{std::make_shared<VmNativeFunction>("resize", 2,
        [handle](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 2)
                throw std::runtime_error("image.resize() takes 2 arguments (width, height).");
            if (!handle->loaded) throw std::runtime_error("image.resize(): image is not loaded.");
            auto newHandle = std::make_shared<VmImageHandle>();
            newHandle->image = ImageCopy(handle->image);
            newHandle->loaded = true;
            ImageResize(&newHandle->image,
                        static_cast<int>(asNumber(args[0])),
                        static_cast<int>(asNumber(args[1])));
            return vmBuildImageObject(newHandle);
        })};

    obj->entries["crop"] = Value{std::make_shared<VmNativeFunction>("crop", 4,
        [handle](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 4)
                throw std::runtime_error("image.crop() takes 4 arguments (x, y, width, height).");
            if (!handle->loaded) throw std::runtime_error("image.crop(): image is not loaded.");
            ::Rectangle rec{
                static_cast<float>(asNumber(args[0])),
                static_cast<float>(asNumber(args[1])),
                static_cast<float>(asNumber(args[2])),
                static_cast<float>(asNumber(args[3]))
            };
            auto newHandle = std::make_shared<VmImageHandle>();
            newHandle->image = ImageCopy(handle->image);
            newHandle->loaded = true;
            ImageCrop(&newHandle->image, rec);
            return vmBuildImageObject(newHandle);
        })};

    obj->entries["rotate"] = Value{std::make_shared<VmNativeFunction>("rotate", 1,
        [handle](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1)
                throw std::runtime_error("image.rotate() takes 1 argument (degrees).");
            if (!handle->loaded) throw std::runtime_error("image.rotate(): image is not loaded.");
            auto newHandle = std::make_shared<VmImageHandle>();
            newHandle->image = ImageCopy(handle->image);
            newHandle->loaded = true;
            ImageRotate(&newHandle->image, static_cast<int>(asNumber(args[0])));
            return vmBuildImageObject(newHandle);
        })};

    obj->entries["flipHorizontal"] = Value{std::make_shared<VmNativeFunction>("flipHorizontal", 0,
        [handle](VM&, const std::vector<Value>&) -> Value {
            if (!handle->loaded)
                throw std::runtime_error("image.flipHorizontal(): image is not loaded.");
            auto newHandle = std::make_shared<VmImageHandle>();
            newHandle->image = ImageCopy(handle->image);
            newHandle->loaded = true;
            ImageFlipHorizontal(&newHandle->image);
            return vmBuildImageObject(newHandle);
        })};

    obj->entries["flipVertical"] = Value{std::make_shared<VmNativeFunction>("flipVertical", 0,
        [handle](VM&, const std::vector<Value>&) -> Value {
            if (!handle->loaded)
                throw std::runtime_error("image.flipVertical(): image is not loaded.");
            auto newHandle = std::make_shared<VmImageHandle>();
            newHandle->image = ImageCopy(handle->image);
            newHandle->loaded = true;
            ImageFlipVertical(&newHandle->image);
            return vmBuildImageObject(newHandle);
        })};

    obj->entries["blur"] = Value{std::make_shared<VmNativeFunction>("blur", 1,
        [handle](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1)
                throw std::runtime_error("image.blur() takes 1 argument (blurSize).");
            if (!handle->loaded) throw std::runtime_error("image.blur(): image is not loaded.");
            auto newHandle = std::make_shared<VmImageHandle>();
            newHandle->image = ImageCopy(handle->image);
            newHandle->loaded = true;
            ImageBlurGaussian(&newHandle->image, static_cast<int>(asNumber(args[0])));
            return vmBuildImageObject(newHandle);
        })};

    obj->entries["save"] = Value{std::make_shared<VmNativeFunction>("save", 1,
        [handle](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1 || !std::holds_alternative<std::string>(args[0]))
                throw std::runtime_error("image.save() takes 1 string argument (path).");
            if (!handle->loaded) throw std::runtime_error("image.save(): image is not loaded.");
            ExportImage(handle->image, std::get<std::string>(args[0]).c_str());
            return Nil{};
        })};

    obj->entries["unload"] = Value{std::make_shared<VmNativeFunction>("unload", 0,
        [handle](VM&, const std::vector<Value>&) -> Value {
            if (handle->loaded) {
                UnloadImage(handle->image);
                handle->loaded = false;
            }
            return Nil{};
        })};

    return Value{obj};
}

#else  // !HAVE_RAYLIB — stb_image-backed implementation

static Value vmBuildImageObject(std::shared_ptr<VmImageHandle> handle) {
    auto obj = std::make_shared<Map>();

    obj->entries["getWidth"] = Value{std::make_shared<VmNativeFunction>("getWidth", 0,
        [handle](VM&, const std::vector<Value>&) -> Value {
            if (!handle->loaded) return 0.0;
            return static_cast<double>(handle->width);
        })};

    obj->entries["getHeight"] = Value{std::make_shared<VmNativeFunction>("getHeight", 0,
        [handle](VM&, const std::vector<Value>&) -> Value {
            if (!handle->loaded) return 0.0;
            return static_cast<double>(handle->height);
        })};

    obj->entries["resize"] = Value{std::make_shared<VmNativeFunction>("resize", 2,
        [handle](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 2)
                throw std::runtime_error("image.resize() takes 2 arguments (width, height).");
            if (!handle->loaded) throw std::runtime_error("image.resize(): image is not loaded.");
            int newW = static_cast<int>(asNumber(args[0]));
            int newH = static_cast<int>(asNumber(args[1]));
            if (newW <= 0 || newH <= 0)
                throw std::runtime_error("image.resize(): width and height must be positive.");
            auto newHandle = std::make_shared<VmImageHandle>();
            newHandle->width = newW;
            newHandle->height = newH;
            newHandle->channels = handle->channels;
            newHandle->pixels.resize(static_cast<size_t>(newW) * newH * handle->channels);
            stbir_resize_uint8_linear(
                handle->pixels.data(), handle->width, handle->height, 0,
                newHandle->pixels.data(), newW, newH, 0,
                static_cast<stbir_pixel_layout>(handle->channels));
            newHandle->loaded = true;
            return vmBuildImageObject(newHandle);
        })};

    obj->entries["crop"] = Value{std::make_shared<VmNativeFunction>("crop", 4,
        [handle](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 4)
                throw std::runtime_error("image.crop() takes 4 arguments (x, y, width, height).");
            if (!handle->loaded) throw std::runtime_error("image.crop(): image is not loaded.");
            int cx = static_cast<int>(asNumber(args[0]));
            int cy = static_cast<int>(asNumber(args[1]));
            int cw = static_cast<int>(asNumber(args[2]));
            int ch = static_cast<int>(asNumber(args[3]));
            if (cx < 0 || cy < 0 || cw <= 0 || ch <= 0 ||
                cx + cw > handle->width || cy + ch > handle->height)
                throw std::runtime_error("image.crop(): crop region out of bounds.");
            auto newHandle = std::make_shared<VmImageHandle>();
            newHandle->width = cw;
            newHandle->height = ch;
            newHandle->channels = handle->channels;
            newHandle->pixels.resize(static_cast<size_t>(cw) * ch * handle->channels);
            for (int row = 0; row < ch; row++) {
                std::memcpy(
                    newHandle->pixels.data() + static_cast<size_t>(row) * cw * handle->channels,
                    handle->pixels.data() +
                        static_cast<size_t>(cy + row) * handle->width * handle->channels +
                        static_cast<size_t>(cx) * handle->channels,
                    static_cast<size_t>(cw) * handle->channels);
            }
            newHandle->loaded = true;
            return vmBuildImageObject(newHandle);
        })};

    obj->entries["rotate"] = Value{std::make_shared<VmNativeFunction>("rotate", 1,
        [handle](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1)
                throw std::runtime_error("image.rotate() takes 1 argument (degrees).");
            if (!handle->loaded) throw std::runtime_error("image.rotate(): image is not loaded.");
            int deg = static_cast<int>(asNumber(args[0]));
            int steps = (((deg % 360) + 360) % 360 + 45) / 90 % 4;
            auto src = handle->pixels;
            int sw = handle->width, sh = handle->height;
            for (int i = 0; i < steps; i++) {
                src = vmRotatePixels90CW(src, sw, sh, handle->channels);
                std::swap(sw, sh);
            }
            auto newHandle = std::make_shared<VmImageHandle>();
            newHandle->pixels = std::move(src);
            newHandle->width = sw;
            newHandle->height = sh;
            newHandle->channels = handle->channels;
            newHandle->loaded = true;
            return vmBuildImageObject(newHandle);
        })};

    obj->entries["flipHorizontal"] = Value{std::make_shared<VmNativeFunction>("flipHorizontal", 0,
        [handle](VM&, const std::vector<Value>&) -> Value {
            if (!handle->loaded)
                throw std::runtime_error("image.flipHorizontal(): image is not loaded.");
            auto newHandle = std::make_shared<VmImageHandle>();
            newHandle->width = handle->width;
            newHandle->height = handle->height;
            newHandle->channels = handle->channels;
            newHandle->pixels.resize(handle->pixels.size());
            int ch = handle->channels;
            for (int y = 0; y < handle->height; y++) {
                for (int x = 0; x < handle->width; x++) {
                    std::memcpy(
                        newHandle->pixels.data() +
                            (static_cast<size_t>(y) * handle->width + (handle->width - 1 - x)) * ch,
                        handle->pixels.data() +
                            (static_cast<size_t>(y) * handle->width + x) * ch,
                        ch);
                }
            }
            newHandle->loaded = true;
            return vmBuildImageObject(newHandle);
        })};

    obj->entries["flipVertical"] = Value{std::make_shared<VmNativeFunction>("flipVertical", 0,
        [handle](VM&, const std::vector<Value>&) -> Value {
            if (!handle->loaded)
                throw std::runtime_error("image.flipVertical(): image is not loaded.");
            auto newHandle = std::make_shared<VmImageHandle>();
            newHandle->width = handle->width;
            newHandle->height = handle->height;
            newHandle->channels = handle->channels;
            newHandle->pixels.resize(handle->pixels.size());
            size_t stride = static_cast<size_t>(handle->width) * handle->channels;
            for (int y = 0; y < handle->height; y++) {
                std::memcpy(
                    newHandle->pixels.data() + static_cast<size_t>(y) * stride,
                    handle->pixels.data() +
                        static_cast<size_t>(handle->height - 1 - y) * stride,
                    stride);
            }
            newHandle->loaded = true;
            return vmBuildImageObject(newHandle);
        })};

    obj->entries["blur"] = Value{std::make_shared<VmNativeFunction>("blur", 1,
        [handle](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1)
                throw std::runtime_error("image.blur() takes 1 argument (blurSize).");
            if (!handle->loaded) throw std::runtime_error("image.blur(): image is not loaded.");
            int radius = static_cast<int>(asNumber(args[0]));
            auto newHandle = std::make_shared<VmImageHandle>();
            newHandle->width = handle->width;
            newHandle->height = handle->height;
            newHandle->channels = handle->channels;
            newHandle->pixels = vmBlurPixels(
                handle->pixels, handle->width, handle->height, handle->channels, radius);
            newHandle->loaded = true;
            return vmBuildImageObject(newHandle);
        })};

    obj->entries["save"] = Value{std::make_shared<VmNativeFunction>("save", 1,
        [handle](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1 || !std::holds_alternative<std::string>(args[0]))
                throw std::runtime_error("image.save() takes 1 string argument (path).");
            if (!handle->loaded) throw std::runtime_error("image.save(): image is not loaded.");
            vmSaveImageToFile(std::get<std::string>(args[0]),
                              handle->pixels.data(), handle->width, handle->height, handle->channels);
            return Nil{};
        })};

    obj->entries["unload"] = Value{std::make_shared<VmNativeFunction>("unload", 0,
        [handle](VM&, const std::vector<Value>&) -> Value {
            handle->pixels.clear();
            handle->pixels.shrink_to_fit();
            handle->loaded = false;
            return Nil{};
        })};

    return Value{obj};
}
#endif  // HAVE_RAYLIB

// ---------------------------------------------------------------------------
// Build the top-level image module Map (VM version)
// ---------------------------------------------------------------------------
Value createVmImageModule(VM& /*vm*/) {
    auto module = std::make_shared<Map>();

    // image.load(path) -> image object
    module->entries["load"] = Value{std::make_shared<VmNativeFunction>("load", 1,
        [](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1 || !std::holds_alternative<std::string>(args[0]))
                throw std::runtime_error("image.load() takes 1 string argument (path).");
            const std::string& path = std::get<std::string>(args[0]);
#ifdef HAVE_RAYLIB
            auto handle = std::make_shared<VmImageHandle>();
            handle->image = LoadImage(path.c_str());
            if (handle->image.data == nullptr)
                throw std::runtime_error("image.load(): failed to load image from '" + path + "'.");
            handle->loaded = true;
            return vmBuildImageObject(handle);
#else
            int w = 0, h = 0, c = 0;
            unsigned char* raw = stbi_load(path.c_str(), &w, &h, &c, 4);
            if (!raw)
                throw std::runtime_error("image.load(): failed to load image from '" + path +
                                         "': " + stbi_failure_reason());
            auto handle = std::make_shared<VmImageHandle>();
            handle->pixels.assign(raw, raw + static_cast<size_t>(w) * h * 4);
            stbi_image_free(raw);
            handle->width = w;
            handle->height = h;
            handle->channels = 4;
            handle->loaded = true;
            return vmBuildImageObject(handle);
#endif
        })};

    return Value{module};
}

}  // namespace izi
