#include "vm_native_audio.hpp"
#include "vm_native.hpp"
#include "vm.hpp"
#include <memory>
#include <string>

#ifdef HAVE_RAYLIB
#include <raylib.h>
#endif

namespace izi {

// ---------------------------------------------------------------------------
// Sound wrapper (holds a raylib Sound handle)
// ---------------------------------------------------------------------------
#ifdef HAVE_RAYLIB
struct VmSoundHandle {
    ::Sound sound{};
    bool loaded = false;
    ~VmSoundHandle() {
        if (loaded) {
            UnloadSound(sound);
            loaded = false;
        }
    }
};

struct VmMusicHandle {
    ::Music music{};
    bool loaded = false;
    ~VmMusicHandle() {
        if (loaded) {
            UnloadMusicStream(music);
            loaded = false;
        }
    }
};
#endif

// ---------------------------------------------------------------------------
// Build a sound object from a loaded sound handle
// ---------------------------------------------------------------------------
#ifdef HAVE_RAYLIB
static Value vmBuildSoundObject(std::shared_ptr<VmSoundHandle> handle) {
    auto obj = std::make_shared<Map>();

    obj->entries["play"] = Value{std::make_shared<VmNativeFunction>("play", 0,
        [handle](VM&, const std::vector<Value>&) -> Value {
            if (handle->loaded) PlaySound(handle->sound);
            return Nil{};
        })};

    obj->entries["stop"] = Value{std::make_shared<VmNativeFunction>("stop", 0,
        [handle](VM&, const std::vector<Value>&) -> Value {
            if (handle->loaded) StopSound(handle->sound);
            return Nil{};
        })};

    obj->entries["pause"] = Value{std::make_shared<VmNativeFunction>("pause", 0,
        [handle](VM&, const std::vector<Value>&) -> Value {
            if (handle->loaded) PauseSound(handle->sound);
            return Nil{};
        })};

    obj->entries["resume"] = Value{std::make_shared<VmNativeFunction>("resume", 0,
        [handle](VM&, const std::vector<Value>&) -> Value {
            if (handle->loaded) ResumeSound(handle->sound);
            return Nil{};
        })};

    obj->entries["isPlaying"] = Value{std::make_shared<VmNativeFunction>("isPlaying", 0,
        [handle](VM&, const std::vector<Value>&) -> Value {
            return handle->loaded && static_cast<bool>(IsSoundPlaying(handle->sound));
        })};

    obj->entries["setVolume"] = Value{std::make_shared<VmNativeFunction>("setVolume", 1,
        [handle](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1) {
                throw std::runtime_error("sound.setVolume() takes 1 argument (volume).");
            }
            if (handle->loaded)
                SetSoundVolume(handle->sound, static_cast<float>(asNumber(args[0])));
            return Nil{};
        })};

    obj->entries["setPitch"] = Value{std::make_shared<VmNativeFunction>("setPitch", 1,
        [handle](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1) {
                throw std::runtime_error("sound.setPitch() takes 1 argument (pitch).");
            }
            if (handle->loaded)
                SetSoundPitch(handle->sound, static_cast<float>(asNumber(args[0])));
            return Nil{};
        })};

    obj->entries["setPan"] = Value{std::make_shared<VmNativeFunction>("setPan", 1,
        [handle](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1) {
                throw std::runtime_error("sound.setPan() takes 1 argument (pan).");
            }
            if (handle->loaded)
                SetSoundPan(handle->sound, static_cast<float>(asNumber(args[0])));
            return Nil{};
        })};

    obj->entries["unload"] = Value{std::make_shared<VmNativeFunction>("unload", 0,
        [handle](VM&, const std::vector<Value>&) -> Value {
            if (handle->loaded) {
                UnloadSound(handle->sound);
                handle->loaded = false;
            }
            return Nil{};
        })};

    return Value{obj};
}
#endif

// ---------------------------------------------------------------------------
// Build a music stream object from a loaded music handle
// ---------------------------------------------------------------------------
#ifdef HAVE_RAYLIB
static Value vmBuildMusicObject(std::shared_ptr<VmMusicHandle> handle) {
    auto obj = std::make_shared<Map>();

    obj->entries["play"] = Value{std::make_shared<VmNativeFunction>("play", 0,
        [handle](VM&, const std::vector<Value>&) -> Value {
            if (handle->loaded) PlayMusicStream(handle->music);
            return Nil{};
        })};

    obj->entries["stop"] = Value{std::make_shared<VmNativeFunction>("stop", 0,
        [handle](VM&, const std::vector<Value>&) -> Value {
            if (handle->loaded) StopMusicStream(handle->music);
            return Nil{};
        })};

    obj->entries["pause"] = Value{std::make_shared<VmNativeFunction>("pause", 0,
        [handle](VM&, const std::vector<Value>&) -> Value {
            if (handle->loaded) PauseMusicStream(handle->music);
            return Nil{};
        })};

    obj->entries["resume"] = Value{std::make_shared<VmNativeFunction>("resume", 0,
        [handle](VM&, const std::vector<Value>&) -> Value {
            if (handle->loaded) ResumeMusicStream(handle->music);
            return Nil{};
        })};

    obj->entries["update"] = Value{std::make_shared<VmNativeFunction>("update", 0,
        [handle](VM&, const std::vector<Value>&) -> Value {
            if (handle->loaded) UpdateMusicStream(handle->music);
            return Nil{};
        })};

    obj->entries["isPlaying"] = Value{std::make_shared<VmNativeFunction>("isPlaying", 0,
        [handle](VM&, const std::vector<Value>&) -> Value {
            return handle->loaded && static_cast<bool>(IsMusicStreamPlaying(handle->music));
        })};

    obj->entries["setVolume"] = Value{std::make_shared<VmNativeFunction>("setVolume", 1,
        [handle](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1) {
                throw std::runtime_error("music.setVolume() takes 1 argument (volume).");
            }
            if (handle->loaded)
                SetMusicVolume(handle->music, static_cast<float>(asNumber(args[0])));
            return Nil{};
        })};

    obj->entries["setPitch"] = Value{std::make_shared<VmNativeFunction>("setPitch", 1,
        [handle](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1) {
                throw std::runtime_error("music.setPitch() takes 1 argument (pitch).");
            }
            if (handle->loaded)
                SetMusicPitch(handle->music, static_cast<float>(asNumber(args[0])));
            return Nil{};
        })};

    obj->entries["setPan"] = Value{std::make_shared<VmNativeFunction>("setPan", 1,
        [handle](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1) {
                throw std::runtime_error("music.setPan() takes 1 argument (pan).");
            }
            if (handle->loaded)
                SetMusicPan(handle->music, static_cast<float>(asNumber(args[0])));
            return Nil{};
        })};

    obj->entries["seek"] = Value{std::make_shared<VmNativeFunction>("seek", 1,
        [handle](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1) {
                throw std::runtime_error("music.seek() takes 1 argument (position).");
            }
            if (handle->loaded)
                SeekMusicStream(handle->music, static_cast<float>(asNumber(args[0])));
            return Nil{};
        })};

    obj->entries["getLength"] = Value{std::make_shared<VmNativeFunction>("getLength", 0,
        [handle](VM&, const std::vector<Value>&) -> Value {
            if (!handle->loaded) return 0.0;
            return static_cast<double>(GetMusicTimeLength(handle->music));
        })};

    obj->entries["getTimePlayed"] = Value{std::make_shared<VmNativeFunction>("getTimePlayed", 0,
        [handle](VM&, const std::vector<Value>&) -> Value {
            if (!handle->loaded) return 0.0;
            return static_cast<double>(GetMusicTimePlayed(handle->music));
        })};

    obj->entries["unload"] = Value{std::make_shared<VmNativeFunction>("unload", 0,
        [handle](VM&, const std::vector<Value>&) -> Value {
            if (handle->loaded) {
                UnloadMusicStream(handle->music);
                handle->loaded = false;
            }
            return Nil{};
        })};

    return Value{obj};
}
#endif

// ---------------------------------------------------------------------------
// Build the top-level audio module Map (VM version)
// ---------------------------------------------------------------------------
Value createVmAudioModule(VM& /*vm*/) {
    auto module = std::make_shared<Map>();

    // audio.initDevice()
    module->entries["initDevice"] = Value{std::make_shared<VmNativeFunction>("initDevice", 0,
        [](VM&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            InitAudioDevice();
#else
            throw std::runtime_error("audio module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    // audio.closeDevice()
    module->entries["closeDevice"] = Value{std::make_shared<VmNativeFunction>("closeDevice", 0,
        [](VM&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            CloseAudioDevice();
#else
            throw std::runtime_error("audio module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    // audio.isDeviceReady() -> bool
    module->entries["isDeviceReady"] = Value{std::make_shared<VmNativeFunction>("isDeviceReady", 0,
        [](VM&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            return static_cast<bool>(IsAudioDeviceReady());
#else
            return false;
#endif
        })};

    // audio.setMasterVolume(volume)
    module->entries["setMasterVolume"] = Value{std::make_shared<VmNativeFunction>("setMasterVolume", 1,
        [](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1) {
                throw std::runtime_error("audio.setMasterVolume() takes 1 argument (volume).");
            }
#ifdef HAVE_RAYLIB
            SetMasterVolume(static_cast<float>(asNumber(args[0])));
#else
            throw std::runtime_error("audio module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    // audio.loadSound(path) -> sound object
    module->entries["loadSound"] = Value{std::make_shared<VmNativeFunction>("loadSound", 1,
        [](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1 || !std::holds_alternative<std::string>(args[0])) {
                throw std::runtime_error("audio.loadSound() takes 1 string argument (path).");
            }
#ifdef HAVE_RAYLIB
            const std::string& path = std::get<std::string>(args[0]);
            auto handle = std::make_shared<VmSoundHandle>();
            handle->sound = LoadSound(path.c_str());
            handle->loaded = true;
            return vmBuildSoundObject(handle);
#else
            throw std::runtime_error("audio module requires raylib (build with -DHAVE_RAYLIB).");
#endif
        })};

    // audio.loadMusic(path) -> music stream object
    module->entries["loadMusic"] = Value{std::make_shared<VmNativeFunction>("loadMusic", 1,
        [](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1 || !std::holds_alternative<std::string>(args[0])) {
                throw std::runtime_error("audio.loadMusic() takes 1 string argument (path).");
            }
#ifdef HAVE_RAYLIB
            const std::string& path = std::get<std::string>(args[0]);
            auto handle = std::make_shared<VmMusicHandle>();
            handle->music = LoadMusicStream(path.c_str());
            handle->loaded = true;
            return vmBuildMusicObject(handle);
#else
            throw std::runtime_error("audio module requires raylib (build with -DHAVE_RAYLIB).");
#endif
        })};

    return Value{module};
}

}  // namespace izi
