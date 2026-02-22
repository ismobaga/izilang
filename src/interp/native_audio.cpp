#include "native_audio.hpp"
#include "native.hpp"
#include "interpreter.hpp"
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
struct SoundHandle {
    ::Sound sound{};
    bool loaded = false;
    ~SoundHandle() {
        if (loaded) {
            UnloadSound(sound);
            loaded = false;
        }
    }
};

struct MusicHandle {
    ::Music music{};
    bool loaded = false;
    ~MusicHandle() {
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
static Value buildSoundObject(std::shared_ptr<SoundHandle> handle) {
    auto obj = std::make_shared<Map>();

    obj->entries["play"] = Value{std::make_shared<NativeFunction>("play", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            if (handle->loaded) PlaySound(handle->sound);
            return Nil{};
        })};

    obj->entries["stop"] = Value{std::make_shared<NativeFunction>("stop", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            if (handle->loaded) StopSound(handle->sound);
            return Nil{};
        })};

    obj->entries["pause"] = Value{std::make_shared<NativeFunction>("pause", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            if (handle->loaded) PauseSound(handle->sound);
            return Nil{};
        })};

    obj->entries["resume"] = Value{std::make_shared<NativeFunction>("resume", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            if (handle->loaded) ResumeSound(handle->sound);
            return Nil{};
        })};

    obj->entries["isPlaying"] = Value{std::make_shared<NativeFunction>("isPlaying", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            return handle->loaded && static_cast<bool>(IsSoundPlaying(handle->sound));
        })};

    obj->entries["setVolume"] = Value{std::make_shared<NativeFunction>("setVolume", 1,
        [handle](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1) {
                throw std::runtime_error("sound.setVolume() takes 1 argument (volume).");
            }
            if (handle->loaded)
                SetSoundVolume(handle->sound, static_cast<float>(asNumber(args[0])));
            return Nil{};
        })};

    obj->entries["setPitch"] = Value{std::make_shared<NativeFunction>("setPitch", 1,
        [handle](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1) {
                throw std::runtime_error("sound.setPitch() takes 1 argument (pitch).");
            }
            if (handle->loaded)
                SetSoundPitch(handle->sound, static_cast<float>(asNumber(args[0])));
            return Nil{};
        })};

    obj->entries["setPan"] = Value{std::make_shared<NativeFunction>("setPan", 1,
        [handle](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1) {
                throw std::runtime_error("sound.setPan() takes 1 argument (pan).");
            }
            if (handle->loaded)
                SetSoundPan(handle->sound, static_cast<float>(asNumber(args[0])));
            return Nil{};
        })};

    obj->entries["unload"] = Value{std::make_shared<NativeFunction>("unload", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
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
static Value buildMusicObject(std::shared_ptr<MusicHandle> handle) {
    auto obj = std::make_shared<Map>();

    obj->entries["play"] = Value{std::make_shared<NativeFunction>("play", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            if (handle->loaded) PlayMusicStream(handle->music);
            return Nil{};
        })};

    obj->entries["stop"] = Value{std::make_shared<NativeFunction>("stop", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            if (handle->loaded) StopMusicStream(handle->music);
            return Nil{};
        })};

    obj->entries["pause"] = Value{std::make_shared<NativeFunction>("pause", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            if (handle->loaded) PauseMusicStream(handle->music);
            return Nil{};
        })};

    obj->entries["resume"] = Value{std::make_shared<NativeFunction>("resume", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            if (handle->loaded) ResumeMusicStream(handle->music);
            return Nil{};
        })};

    obj->entries["update"] = Value{std::make_shared<NativeFunction>("update", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            if (handle->loaded) UpdateMusicStream(handle->music);
            return Nil{};
        })};

    obj->entries["isPlaying"] = Value{std::make_shared<NativeFunction>("isPlaying", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            return handle->loaded && static_cast<bool>(IsMusicStreamPlaying(handle->music));
        })};

    obj->entries["setVolume"] = Value{std::make_shared<NativeFunction>("setVolume", 1,
        [handle](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1) {
                throw std::runtime_error("music.setVolume() takes 1 argument (volume).");
            }
            if (handle->loaded)
                SetMusicVolume(handle->music, static_cast<float>(asNumber(args[0])));
            return Nil{};
        })};

    obj->entries["setPitch"] = Value{std::make_shared<NativeFunction>("setPitch", 1,
        [handle](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1) {
                throw std::runtime_error("music.setPitch() takes 1 argument (pitch).");
            }
            if (handle->loaded)
                SetMusicPitch(handle->music, static_cast<float>(asNumber(args[0])));
            return Nil{};
        })};

    obj->entries["setPan"] = Value{std::make_shared<NativeFunction>("setPan", 1,
        [handle](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1) {
                throw std::runtime_error("music.setPan() takes 1 argument (pan).");
            }
            if (handle->loaded)
                SetMusicPan(handle->music, static_cast<float>(asNumber(args[0])));
            return Nil{};
        })};

    obj->entries["seek"] = Value{std::make_shared<NativeFunction>("seek", 1,
        [handle](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1) {
                throw std::runtime_error("music.seek() takes 1 argument (position).");
            }
            if (handle->loaded)
                SeekMusicStream(handle->music, static_cast<float>(asNumber(args[0])));
            return Nil{};
        })};

    obj->entries["getLength"] = Value{std::make_shared<NativeFunction>("getLength", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            if (!handle->loaded) return 0.0;
            return static_cast<double>(GetMusicTimeLength(handle->music));
        })};

    obj->entries["getTimePlayed"] = Value{std::make_shared<NativeFunction>("getTimePlayed", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            if (!handle->loaded) return 0.0;
            return static_cast<double>(GetMusicTimePlayed(handle->music));
        })};

    obj->entries["unload"] = Value{std::make_shared<NativeFunction>("unload", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
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
// Build the top-level audio module Map
// ---------------------------------------------------------------------------
Value createAudioModule(Interpreter& /*interp*/) {
    auto module = std::make_shared<Map>();

    // audio.initDevice()
    module->entries["initDevice"] = Value{std::make_shared<NativeFunction>("initDevice", 0,
        [](Interpreter&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            InitAudioDevice();
#else
            throw std::runtime_error("audio module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    // audio.closeDevice()
    module->entries["closeDevice"] = Value{std::make_shared<NativeFunction>("closeDevice", 0,
        [](Interpreter&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            CloseAudioDevice();
#else
            throw std::runtime_error("audio module requires raylib (build with -DHAVE_RAYLIB).");
#endif
            return Nil{};
        })};

    // audio.isDeviceReady() -> bool
    module->entries["isDeviceReady"] = Value{std::make_shared<NativeFunction>("isDeviceReady", 0,
        [](Interpreter&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            return static_cast<bool>(IsAudioDeviceReady());
#else
            return false;
#endif
        })};

    // audio.setMasterVolume(volume)
    module->entries["setMasterVolume"] = Value{std::make_shared<NativeFunction>("setMasterVolume", 1,
        [](Interpreter&, const std::vector<Value>& args) -> Value {
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
    module->entries["loadSound"] = Value{std::make_shared<NativeFunction>("loadSound", 1,
        [](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1 || !std::holds_alternative<std::string>(args[0])) {
                throw std::runtime_error("audio.loadSound() takes 1 string argument (path).");
            }
#ifdef HAVE_RAYLIB
            const std::string& path = std::get<std::string>(args[0]);
            auto handle = std::make_shared<SoundHandle>();
            handle->sound = LoadSound(path.c_str());
            handle->loaded = true;
            return buildSoundObject(handle);
#else
            throw std::runtime_error("audio module requires raylib (build with -DHAVE_RAYLIB).");
#endif
        })};

    // audio.loadMusic(path) -> music stream object
    module->entries["loadMusic"] = Value{std::make_shared<NativeFunction>("loadMusic", 1,
        [](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1 || !std::holds_alternative<std::string>(args[0])) {
                throw std::runtime_error("audio.loadMusic() takes 1 string argument (path).");
            }
#ifdef HAVE_RAYLIB
            const std::string& path = std::get<std::string>(args[0]);
            auto handle = std::make_shared<MusicHandle>();
            handle->music = LoadMusicStream(path.c_str());
            handle->loaded = true;
            return buildMusicObject(handle);
#else
            throw std::runtime_error("audio module requires raylib (build with -DHAVE_RAYLIB).");
#endif
        })};

    return Value{module};
}

}  // namespace izi
