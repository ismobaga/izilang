#include "native_image.hpp"
#include "native.hpp"
#include "interpreter.hpp"
#include <memory>
#include <string>

#ifdef HAVE_RAYLIB
#include <raylib.h>
#endif

namespace izi {

// ---------------------------------------------------------------------------
// Image wrapper (holds a raylib Image handle)
// ---------------------------------------------------------------------------
#ifdef HAVE_RAYLIB
struct ImageHandle {
    ::Image image{};
    bool loaded = false;
    ~ImageHandle() {
        if (loaded) {
            UnloadImage(image);
            loaded = false;
        }
    }
};
#endif

// ---------------------------------------------------------------------------
// Build an image object from a loaded image handle
// ---------------------------------------------------------------------------
#ifdef HAVE_RAYLIB
static Value buildImageObject(std::shared_ptr<ImageHandle> handle) {
    auto obj = std::make_shared<Map>();

    obj->entries["getWidth"] = Value{std::make_shared<NativeFunction>("getWidth", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            if (!handle->loaded) return 0.0;
            return static_cast<double>(handle->image.width);
        })};

    obj->entries["getHeight"] = Value{std::make_shared<NativeFunction>("getHeight", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            if (!handle->loaded) return 0.0;
            return static_cast<double>(handle->image.height);
        })};

    obj->entries["resize"] = Value{std::make_shared<NativeFunction>("resize", 2,
        [handle](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 2) {
                throw std::runtime_error("image.resize() takes 2 arguments (width, height).");
            }
            if (!handle->loaded) throw std::runtime_error("image.resize(): image is not loaded.");
            auto newHandle = std::make_shared<ImageHandle>();
            newHandle->image = ImageCopy(handle->image);
            newHandle->loaded = true;
            ImageResize(&newHandle->image,
                        static_cast<int>(asNumber(args[0])),
                        static_cast<int>(asNumber(args[1])));
            return buildImageObject(newHandle);
        })};

    obj->entries["crop"] = Value{std::make_shared<NativeFunction>("crop", 4,
        [handle](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 4) {
                throw std::runtime_error("image.crop() takes 4 arguments (x, y, width, height).");
            }
            if (!handle->loaded) throw std::runtime_error("image.crop(): image is not loaded.");
            ::Rectangle rec{
                static_cast<float>(asNumber(args[0])),
                static_cast<float>(asNumber(args[1])),
                static_cast<float>(asNumber(args[2])),
                static_cast<float>(asNumber(args[3]))
            };
            auto newHandle = std::make_shared<ImageHandle>();
            newHandle->image = ImageCopy(handle->image);
            newHandle->loaded = true;
            ImageCrop(&newHandle->image, rec);
            return buildImageObject(newHandle);
        })};

    obj->entries["rotate"] = Value{std::make_shared<NativeFunction>("rotate", 1,
        [handle](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1) {
                throw std::runtime_error("image.rotate() takes 1 argument (degrees).");
            }
            if (!handle->loaded) throw std::runtime_error("image.rotate(): image is not loaded.");
            auto newHandle = std::make_shared<ImageHandle>();
            newHandle->image = ImageCopy(handle->image);
            newHandle->loaded = true;
            ImageRotate(&newHandle->image, static_cast<int>(asNumber(args[0])));
            return buildImageObject(newHandle);
        })};

    obj->entries["flipHorizontal"] = Value{std::make_shared<NativeFunction>("flipHorizontal", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            if (!handle->loaded) throw std::runtime_error("image.flipHorizontal(): image is not loaded.");
            auto newHandle = std::make_shared<ImageHandle>();
            newHandle->image = ImageCopy(handle->image);
            newHandle->loaded = true;
            ImageFlipHorizontal(&newHandle->image);
            return buildImageObject(newHandle);
        })};

    obj->entries["flipVertical"] = Value{std::make_shared<NativeFunction>("flipVertical", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            if (!handle->loaded) throw std::runtime_error("image.flipVertical(): image is not loaded.");
            auto newHandle = std::make_shared<ImageHandle>();
            newHandle->image = ImageCopy(handle->image);
            newHandle->loaded = true;
            ImageFlipVertical(&newHandle->image);
            return buildImageObject(newHandle);
        })};

    obj->entries["blur"] = Value{std::make_shared<NativeFunction>("blur", 1,
        [handle](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1) {
                throw std::runtime_error("image.blur() takes 1 argument (blurSize).");
            }
            if (!handle->loaded) throw std::runtime_error("image.blur(): image is not loaded.");
            auto newHandle = std::make_shared<ImageHandle>();
            newHandle->image = ImageCopy(handle->image);
            newHandle->loaded = true;
            ImageBlurGaussian(&newHandle->image, static_cast<int>(asNumber(args[0])));
            return buildImageObject(newHandle);
        })};

    obj->entries["save"] = Value{std::make_shared<NativeFunction>("save", 1,
        [handle](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1 || !std::holds_alternative<std::string>(args[0])) {
                throw std::runtime_error("image.save() takes 1 string argument (path).");
            }
            if (!handle->loaded) throw std::runtime_error("image.save(): image is not loaded.");
            ExportImage(handle->image, std::get<std::string>(args[0]).c_str());
            return Nil{};
        })};

    obj->entries["unload"] = Value{std::make_shared<NativeFunction>("unload", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            if (handle->loaded) {
                UnloadImage(handle->image);
                handle->loaded = false;
            }
            return Nil{};
        })};

    return Value{obj};
}
#endif

// ---------------------------------------------------------------------------
// Build the top-level image module Map
// ---------------------------------------------------------------------------
Value createImageModule(Interpreter& /*interp*/) {
    auto module = std::make_shared<Map>();

    // image.load(path) -> image object
    module->entries["load"] = Value{std::make_shared<NativeFunction>("load", 1,
        [](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1 || !std::holds_alternative<std::string>(args[0])) {
                throw std::runtime_error("image.load() takes 1 string argument (path).");
            }
#ifdef HAVE_RAYLIB
            const std::string& path = std::get<std::string>(args[0]);
            auto handle = std::make_shared<ImageHandle>();
            handle->image = LoadImage(path.c_str());
            if (handle->image.data == nullptr) {
                throw std::runtime_error("image.load(): failed to load image from '" + path + "'.");
            }
            handle->loaded = true;
            return buildImageObject(handle);
#else
            throw std::runtime_error("image module requires raylib (build with -DHAVE_RAYLIB).");
#endif
        })};

    return Value{module};
}

}  // namespace izi
