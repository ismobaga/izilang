#include "vm_native_audio.hpp"
#include "vm_native.hpp"
#include "vm.hpp"
#include <memory>
#include <string>

#ifdef HAVE_RAYLIB
#include <raylib.h>
#else
#include "../vendor/miniaudio.h"
#endif

namespace izi {

// ---------------------------------------------------------------------------
// Audio handles
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
#else
// miniaudio-backed handles for the VM path.
// Re-use the same global engine as the interpreter path via a shared declaration.
struct VmMaEngineState {
    ma_engine engine{};
    bool initialized = false;
    float pendingVolume = 1.0f;

    ~VmMaEngineState() {
        if (initialized) {
            ma_engine_uninit(&engine);
            initialized = false;
        }
    }
};

static VmMaEngineState& getVmAudioEngine() {
    static VmMaEngineState s;
    return s;
}

struct VmSoundHandle {
    ma_sound sound{};
    bool loaded = false;
    ~VmSoundHandle() {
        if (loaded) {
            ma_sound_uninit(&sound);
            loaded = false;
        }
    }
};

struct VmMusicHandle {
    ma_sound sound{};
    bool loaded = false;
    ~VmMusicHandle() {
        if (loaded) {
            ma_sound_uninit(&sound);
            loaded = false;
        }
    }
};
#endif

// ---------------------------------------------------------------------------
// Build a sound object
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
            if (args.size() != 1)
                throw std::runtime_error("sound.setVolume() takes 1 argument (volume).");
            if (handle->loaded)
                SetSoundVolume(handle->sound, static_cast<float>(asNumber(args[0])));
            return Nil{};
        })};

    obj->entries["setPitch"] = Value{std::make_shared<VmNativeFunction>("setPitch", 1,
        [handle](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1)
                throw std::runtime_error("sound.setPitch() takes 1 argument (pitch).");
            if (handle->loaded)
                SetSoundPitch(handle->sound, static_cast<float>(asNumber(args[0])));
            return Nil{};
        })};

    obj->entries["setPan"] = Value{std::make_shared<VmNativeFunction>("setPan", 1,
        [handle](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1)
                throw std::runtime_error("sound.setPan() takes 1 argument (pan).");
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
#else
static Value vmBuildSoundObject(std::shared_ptr<VmSoundHandle> handle) {
    auto obj = std::make_shared<Map>();

    obj->entries["play"] = Value{std::make_shared<VmNativeFunction>("play", 0,
        [handle](VM&, const std::vector<Value>&) -> Value {
            if (handle->loaded) ma_sound_start(&handle->sound);
            return Nil{};
        })};

    obj->entries["stop"] = Value{std::make_shared<VmNativeFunction>("stop", 0,
        [handle](VM&, const std::vector<Value>&) -> Value {
            if (handle->loaded) ma_sound_stop(&handle->sound);
            return Nil{};
        })};

    obj->entries["pause"] = Value{std::make_shared<VmNativeFunction>("pause", 0,
        [handle](VM&, const std::vector<Value>&) -> Value {
            if (handle->loaded) ma_sound_stop(&handle->sound);
            return Nil{};
        })};

    obj->entries["resume"] = Value{std::make_shared<VmNativeFunction>("resume", 0,
        [handle](VM&, const std::vector<Value>&) -> Value {
            if (handle->loaded) ma_sound_start(&handle->sound);
            return Nil{};
        })};

    obj->entries["isPlaying"] = Value{std::make_shared<VmNativeFunction>("isPlaying", 0,
        [handle](VM&, const std::vector<Value>&) -> Value {
            return handle->loaded && static_cast<bool>(ma_sound_is_playing(&handle->sound));
        })};

    obj->entries["setVolume"] = Value{std::make_shared<VmNativeFunction>("setVolume", 1,
        [handle](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1)
                throw std::runtime_error("sound.setVolume() takes 1 argument (volume).");
            if (handle->loaded)
                ma_sound_set_volume(&handle->sound, static_cast<float>(asNumber(args[0])));
            return Nil{};
        })};

    obj->entries["setPitch"] = Value{std::make_shared<VmNativeFunction>("setPitch", 1,
        [handle](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1)
                throw std::runtime_error("sound.setPitch() takes 1 argument (pitch).");
            if (handle->loaded)
                ma_sound_set_pitch(&handle->sound, static_cast<float>(asNumber(args[0])));
            return Nil{};
        })};

    obj->entries["setPan"] = Value{std::make_shared<VmNativeFunction>("setPan", 1,
        [handle](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1)
                throw std::runtime_error("sound.setPan() takes 1 argument (pan).");
            if (handle->loaded)
                ma_sound_set_pan(&handle->sound, static_cast<float>(asNumber(args[0])));
            return Nil{};
        })};

    obj->entries["unload"] = Value{std::make_shared<VmNativeFunction>("unload", 0,
        [handle](VM&, const std::vector<Value>&) -> Value {
            if (handle->loaded) {
                ma_sound_uninit(&handle->sound);
                handle->loaded = false;
            }
            return Nil{};
        })};

    return Value{obj};
}
#endif

// ---------------------------------------------------------------------------
// Build a music stream object
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
            if (args.size() != 1)
                throw std::runtime_error("music.setVolume() takes 1 argument (volume).");
            if (handle->loaded)
                SetMusicVolume(handle->music, static_cast<float>(asNumber(args[0])));
            return Nil{};
        })};

    obj->entries["setPitch"] = Value{std::make_shared<VmNativeFunction>("setPitch", 1,
        [handle](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1)
                throw std::runtime_error("music.setPitch() takes 1 argument (pitch).");
            if (handle->loaded)
                SetMusicPitch(handle->music, static_cast<float>(asNumber(args[0])));
            return Nil{};
        })};

    obj->entries["setPan"] = Value{std::make_shared<VmNativeFunction>("setPan", 1,
        [handle](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1)
                throw std::runtime_error("music.setPan() takes 1 argument (pan).");
            if (handle->loaded)
                SetMusicPan(handle->music, static_cast<float>(asNumber(args[0])));
            return Nil{};
        })};

    obj->entries["seek"] = Value{std::make_shared<VmNativeFunction>("seek", 1,
        [handle](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1)
                throw std::runtime_error("music.seek() takes 1 argument (position).");
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
#else
static Value vmBuildMusicObject(std::shared_ptr<VmMusicHandle> handle) {
    auto obj = std::make_shared<Map>();

    obj->entries["play"] = Value{std::make_shared<VmNativeFunction>("play", 0,
        [handle](VM&, const std::vector<Value>&) -> Value {
            if (handle->loaded) ma_sound_start(&handle->sound);
            return Nil{};
        })};

    obj->entries["stop"] = Value{std::make_shared<VmNativeFunction>("stop", 0,
        [handle](VM&, const std::vector<Value>&) -> Value {
            if (handle->loaded) ma_sound_stop(&handle->sound);
            return Nil{};
        })};

    obj->entries["pause"] = Value{std::make_shared<VmNativeFunction>("pause", 0,
        [handle](VM&, const std::vector<Value>&) -> Value {
            if (handle->loaded) ma_sound_stop(&handle->sound);
            return Nil{};
        })};

    obj->entries["resume"] = Value{std::make_shared<VmNativeFunction>("resume", 0,
        [handle](VM&, const std::vector<Value>&) -> Value {
            if (handle->loaded) ma_sound_start(&handle->sound);
            return Nil{};
        })};

    obj->entries["update"] = Value{std::make_shared<VmNativeFunction>("update", 0,
        [](VM&, const std::vector<Value>&) -> Value {
            // miniaudio streams are updated automatically on a background thread.
            return Nil{};
        })};

    obj->entries["isPlaying"] = Value{std::make_shared<VmNativeFunction>("isPlaying", 0,
        [handle](VM&, const std::vector<Value>&) -> Value {
            return handle->loaded && static_cast<bool>(ma_sound_is_playing(&handle->sound));
        })};

    obj->entries["setVolume"] = Value{std::make_shared<VmNativeFunction>("setVolume", 1,
        [handle](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1)
                throw std::runtime_error("music.setVolume() takes 1 argument (volume).");
            if (handle->loaded)
                ma_sound_set_volume(&handle->sound, static_cast<float>(asNumber(args[0])));
            return Nil{};
        })};

    obj->entries["setPitch"] = Value{std::make_shared<VmNativeFunction>("setPitch", 1,
        [handle](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1)
                throw std::runtime_error("music.setPitch() takes 1 argument (pitch).");
            if (handle->loaded)
                ma_sound_set_pitch(&handle->sound, static_cast<float>(asNumber(args[0])));
            return Nil{};
        })};

    obj->entries["setPan"] = Value{std::make_shared<VmNativeFunction>("setPan", 1,
        [handle](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1)
                throw std::runtime_error("music.setPan() takes 1 argument (pan).");
            if (handle->loaded)
                ma_sound_set_pan(&handle->sound, static_cast<float>(asNumber(args[0])));
            return Nil{};
        })};

    obj->entries["seek"] = Value{std::make_shared<VmNativeFunction>("seek", 1,
        [handle](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1)
                throw std::runtime_error("music.seek() takes 1 argument (position).");
            if (handle->loaded)
                ma_sound_seek_to_pcm_frame(&handle->sound,
                    static_cast<ma_uint64>(asNumber(args[0]) *
                        ma_engine_get_sample_rate(&getVmAudioEngine().engine)));
            return Nil{};
        })};

    obj->entries["getLength"] = Value{std::make_shared<VmNativeFunction>("getLength", 0,
        [handle](VM&, const std::vector<Value>&) -> Value {
            if (!handle->loaded) return 0.0;
            float len = 0.0f;
            ma_sound_get_length_in_seconds(&handle->sound, &len);
            return static_cast<double>(len);
        })};

    obj->entries["getTimePlayed"] = Value{std::make_shared<VmNativeFunction>("getTimePlayed", 0,
        [handle](VM&, const std::vector<Value>&) -> Value {
            if (!handle->loaded) return 0.0;
            float t = 0.0f;
            ma_sound_get_cursor_in_seconds(&handle->sound, &t);
            return static_cast<double>(t);
        })};

    obj->entries["unload"] = Value{std::make_shared<VmNativeFunction>("unload", 0,
        [handle](VM&, const std::vector<Value>&) -> Value {
            if (handle->loaded) {
                ma_sound_uninit(&handle->sound);
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
            auto& eng = getVmAudioEngine();
            if (eng.initialized) return Nil{};
            ma_result result = ma_engine_init(nullptr, &eng.engine);
            if (result != MA_SUCCESS)
                throw std::runtime_error(
                    "audio.initDevice(): failed to initialize audio device (code " +
                    std::to_string(result) + ").");
            eng.initialized = true;
            ma_engine_set_volume(&eng.engine, eng.pendingVolume);
#endif
            return Nil{};
        })};

    // audio.closeDevice()
    module->entries["closeDevice"] = Value{std::make_shared<VmNativeFunction>("closeDevice", 0,
        [](VM&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            CloseAudioDevice();
#else
            auto& eng = getVmAudioEngine();
            if (eng.initialized) {
                ma_engine_uninit(&eng.engine);
                eng.initialized = false;
            }
#endif
            return Nil{};
        })};

    // audio.isDeviceReady() -> bool
    module->entries["isDeviceReady"] = Value{std::make_shared<VmNativeFunction>("isDeviceReady", 0,
        [](VM&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            return static_cast<bool>(IsAudioDeviceReady());
#else
            return getVmAudioEngine().initialized;
#endif
        })};

    // audio.setMasterVolume(volume)
    module->entries["setMasterVolume"] = Value{std::make_shared<VmNativeFunction>("setMasterVolume", 1,
        [](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1)
                throw std::runtime_error("audio.setMasterVolume() takes 1 argument (volume).");
#ifdef HAVE_RAYLIB
            SetMasterVolume(static_cast<float>(asNumber(args[0])));
#else
            auto& eng = getVmAudioEngine();
            float vol = static_cast<float>(asNumber(args[0]));
            eng.pendingVolume = vol;
            if (eng.initialized)
                ma_engine_set_volume(&eng.engine, vol);
#endif
            return Nil{};
        })};

    // audio.loadSound(path) -> sound object
    module->entries["loadSound"] = Value{std::make_shared<VmNativeFunction>("loadSound", 1,
        [](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1 || !std::holds_alternative<std::string>(args[0]))
                throw std::runtime_error("audio.loadSound() takes 1 string argument (path).");
            const std::string& path = std::get<std::string>(args[0]);
#ifdef HAVE_RAYLIB
            auto handle = std::make_shared<VmSoundHandle>();
            handle->sound = LoadSound(path.c_str());
            handle->loaded = true;
            return vmBuildSoundObject(handle);
#else
            auto& eng = getVmAudioEngine();
            if (!eng.initialized)
                throw std::runtime_error(
                    "audio.loadSound(): audio device not initialized. Call audio.initDevice() first.");
            auto handle = std::make_shared<VmSoundHandle>();
            ma_result result = ma_sound_init_from_file(
                &eng.engine, path.c_str(), MA_SOUND_FLAG_DECODE, nullptr, nullptr, &handle->sound);
            if (result != MA_SUCCESS)
                throw std::runtime_error(
                    "audio.loadSound(): failed to load sound from '" + path + "' (code " +
                    std::to_string(result) + ").");
            handle->loaded = true;
            return vmBuildSoundObject(handle);
#endif
        })};

    // audio.loadMusic(path) -> music stream object
    module->entries["loadMusic"] = Value{std::make_shared<VmNativeFunction>("loadMusic", 1,
        [](VM&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1 || !std::holds_alternative<std::string>(args[0]))
                throw std::runtime_error("audio.loadMusic() takes 1 string argument (path).");
            const std::string& path = std::get<std::string>(args[0]);
#ifdef HAVE_RAYLIB
            auto handle = std::make_shared<VmMusicHandle>();
            handle->music = LoadMusicStream(path.c_str());
            handle->loaded = true;
            return vmBuildMusicObject(handle);
#else
            auto& eng = getVmAudioEngine();
            if (!eng.initialized)
                throw std::runtime_error(
                    "audio.loadMusic(): audio device not initialized. Call audio.initDevice() first.");
            auto handle = std::make_shared<VmMusicHandle>();
            ma_result result = ma_sound_init_from_file(
                &eng.engine, path.c_str(), 0, nullptr, nullptr, &handle->sound);
            if (result != MA_SUCCESS)
                throw std::runtime_error(
                    "audio.loadMusic(): failed to load music from '" + path + "' (code " +
                    std::to_string(result) + ").");
            handle->loaded = true;
            return vmBuildMusicObject(handle);
#endif
        })};

    return Value{module};
}

}  // namespace izi
